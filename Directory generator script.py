import os # Funciones del sistema operativo (rutas, directorios, archivos)
import pathlib # Manejo moderno de rutas y sistema de archivos

# VARIABLES GLOBALES
codificacion_archivo = 'utf-8'

# FUNCIONES
# Procesa el contenido de un archivo y lo escribe en su ruta correspondiente
def procesar_contenido_archivo(lineas_contenido, ruta_archivo_destino):
    # Crear directorio padre si no existe
    ruta_archivo_destino.parent.mkdir(parents = True, exist_ok = True)
    
    # Escribir contenido en el archivo
    with open(ruta_archivo_destino, 'w', encoding = codificacion_archivo) as archivo_val:
        archivo_val.write(lineas_contenido)

# Determina si un elemento es archivo (tiene extensión) o directorio
def es_archivo(nombre_elemento):
    # Si tiene un punto y el texto después del punto no está vacío, es archivo
    if '.' in nombre_elemento:
        partes = nombre_elemento.split('.')
        # Verificar que haya al menos 2 partes y la última no esté vacía
        if len(partes) >= 2 and partes[-1].strip():
            return True
    return False

# Procesa estructura de directorios y archivos del árbol
def procesar_estructura_directorios(lineas_arbol, nombre_carpeta_raiz):
    estructura_directorios = [nombre_carpeta_raiz]
    estructura_archivos = []
    
    # Usaremos una pila para mantener la ruta actual según el nivel de indentación
    pila_rutas = [nombre_carpeta_raiz]
    pila_niveles = [0]
    
    for linea_val in lineas_arbol:
        linea_val = linea_val.rstrip('\n\r')
        
        # Saltar líneas vacías
        if not linea_val.strip():
            continue
        
        # Detectar si es línea de árbol (contiene caracteres de árbol)
        tiene_patron_arbol = any(caracter in linea_val for caracter in ['├', '└', '──', '│'])
        
        if tiene_patron_arbol:
            # Calcular nivel por indentación (contar prefijo de árbol)
            nivel_actual = 0
            caracteres_arbol = 0
            
            # Contar caracteres de árbol antes del nombre
            for char in linea_val:
                if char in ['├', '└', '─', '│', ' ']:
                    if char == ' ':
                        caracteres_arbol += 1
                    else:
                        caracteres_arbol += 1
                else:
                    break
            
            # Calcular nivel basado en caracteres de árbol
            nivel_actual = caracteres_arbol // 4
            
            # Extraer nombre del elemento (eliminar caracteres de árbol)
            nombre_elemento = ""
            en_nombre = False
            
            for char in linea_val:
                if not en_nombre and char not in ['├', '└', '─', '│', ' ']:
                    en_nombre = True
                if en_nombre:
                    nombre_elemento += char
            
            nombre_elemento = nombre_elemento.strip()
            
            if not nombre_elemento:
                continue
            
            # Si el nivel actual es menor o igual al anterior, retroceder en la pila
            while len(pila_niveles) > 1 and pila_niveles[-1] >= nivel_actual:
                pila_rutas.pop()
                pila_niveles.pop()
            
            # Obtener la ruta padre actual
            ruta_padre = pila_rutas[-1] if pila_rutas else nombre_carpeta_raiz
            
            # Construir la ruta completa
            ruta_completa = os.path.join(ruta_padre, nombre_elemento)
            
            # Determinar si es directorio o archivo
            if es_archivo(nombre_elemento):
                # Es un archivo
                estructura_archivos.append(ruta_completa)
            else:
                # Es un directorio
                estructura_directorios.append(ruta_completa)
                pila_rutas.append(ruta_completa)
                pila_niveles.append(nivel_actual)
    
    return estructura_directorios, estructura_archivos

# Extrae y procesa los contenidos de los archivos del documento
def extraer_contenidos_archivos(lineas_documento):
    contenidos_archivos = {}
    archivo_actual = None
    contenido_actual = []
    en_contenido_archivo = False
    
    for linea_val in lineas_documento:
        linea_limpia = linea_val.rstrip('\n\r')
        
        # Detectar separadores
        if '------------------------------------' in linea_limpia:
            # Guardar archivo anterior si existe
            if archivo_actual is not None:
                # Unir contenido y guardar (puede estar vacío)
                contenido_final = '\n'.join(contenido_actual).rstrip()
                contenidos_archivos[archivo_actual] = contenido_final
                archivo_actual = None
                contenido_actual = []
                en_contenido_archivo = False
            continue
        
        # Detectar nuevo archivo (línea que contiene ruta de archivo con extensión)
        if (archivo_actual is None and 
            '.' in linea_limpia and 
            not any(caracter in linea_limpia for caracter in ['├', '└', '│', '─']) and
            linea_limpia.strip() == linea_limpia):
            
            archivo_actual = linea_limpia
            contenido_actual = []
            en_contenido_archivo = True
            continue
        
        # Agregar contenido al archivo actual si estamos en sección de contenido
        if en_contenido_archivo and archivo_actual is not None:
            contenido_actual.append(linea_limpia)
    
    # Guardar último archivo procesado
    if archivo_actual is not None:
        contenido_final = '\n'.join(contenido_actual).rstrip()
        contenidos_archivos[archivo_actual] = contenido_final
    
    return contenidos_archivos

# Procesa el archivo de estructura y genera los directorios y archivos
def procesar_archivo_estructura(ruta_archivo):
    # Leer todo el contenido del archivo
    with open(ruta_archivo, 'r', encoding = codificacion_archivo) as archivo_val:
        lineas = archivo_val.readlines()
    
    if not lineas:
        print("Empty file\n")
        return False
    
    # Extraer nombre de carpeta raiz (primera línea)
    nombre_carpeta_raiz = lineas[0].strip()
    
    if not nombre_carpeta_raiz:
        print("No root folder name found\n")
        return False
    
    # Separar secciones
    lineas_arbol = []
    lineas_contenidos = []
    en_contenidos = False
    
    for linea_val in lineas[1:]:
        linea_limpia = linea_val.rstrip('\n\r')
        
        if '------------------------------------' in linea_limpia:
            en_contenidos = True
        
        if en_contenidos:
            lineas_contenidos.append(linea_val)
        else:
            lineas_arbol.append(linea_val)
    
    # Procesar estructura
    directorios, archivos = procesar_estructura_directorios(lineas_arbol, nombre_carpeta_raiz)
    contenidos = extraer_contenidos_archivos(lineas_contenidos)
    
    # Crear directorios
    for directorio_val in directorios:
        pathlib.Path(directorio_val).mkdir(parents = True, exist_ok = True)
    
    # Crear archivos con sus contenidos
    for archivo_ruta in archivos:
        nombre_archivo = os.path.basename(archivo_ruta)
        
        # Buscar contenido por nombre de archivo
        contenido_encontrado = None
        
        for clave_archivo in contenidos:
            if clave_archivo.endswith(nombre_archivo):
                contenido_encontrado = contenidos[clave_archivo]
                break
        
        if contenido_encontrado is not None:
            procesar_contenido_archivo(contenido_encontrado, pathlib.Path(archivo_ruta))
        else:
            # Crear archivo vacío si no se encuentra contenido
            procesar_contenido_archivo("", pathlib.Path(archivo_ruta))
    
    # Mostrar estadísticas
    print(f"Output: {len(directorios)} directories, {len(archivos)} files\n")
    return True

# BUCLE PRINCIPAL
while True:
    # Solicitar directorio del archivo TXT
    directorio_entrada = input("TXT file directory: ").strip('"\'')
    
    # Verificar que el directorio existe
    if not pathlib.Path(directorio_entrada).exists():
        print("Wrong directory\n")
        continue
    
    # Ejecutar procesamiento de estructura
    resultado_procesamiento = procesar_archivo_estructura(directorio_entrada)
    
    # Si el procesamiento falla, mostrar mensaje
    if not resultado_procesamiento:
        print("Processing failed\n")
