#include <iostream> // Entrada/salida estándar
#include <fstream> // Manejo de archivos
#include <filesystem> // Sistema de archivos y rutas
#include <vector> // Arreglos dinámicos
#include <string> // Cadenas de texto
#include <stack> // Estructura pila para navegación

// VARIABLES GLOBALES
const std::string codificacion_archivo = "utf-8";

// FUNCIONES
// Procesa el contenido de un archivo y lo escribe en su ruta correspondiente
void procesar_contenido_archivo(const std::string& lineas_contenido, const std::filesystem::path& ruta_archivo_destino)
{
    // Crear directorio padre si no existe
    std::filesystem::create_directories(ruta_archivo_destino.parent_path());
    
    // Escribir contenido en el archivo
    std::ofstream archivo_val(ruta_archivo_destino);
    
    if (archivo_val.is_open())
    {
        archivo_val << lineas_contenido;
        archivo_val.close();
    }
}

// Determina si un elemento es archivo (tiene extensión) o directorio
bool es_archivo(const std::string& nombre_elemento)
{
    // Si tiene un punto y el texto después del punto no está vacío, es archivo
    if (nombre_elemento.find('.') != std::string::npos)
    {
        size_t pos_punto = nombre_elemento.find_last_of('.');
        
        // Verificar que haya texto después del punto
        if (pos_punto != std::string::npos && pos_punto < nombre_elemento.length() - 1)
        {
            return true;
        }
    }
    
    return false;
}

// Procesa estructura de directorios y archivos del árbol
std::pair<std::vector<std::filesystem::path>, std::vector<std::filesystem::path>> procesar_estructura_directorios(
    const std::vector<std::string>& lineas_arbol, 
    const std::string& nombre_carpeta_raiz, 
    const std::filesystem::path& directorio_base)
{
    std::vector<std::filesystem::path> estructura_directorios;
    std::vector<std::filesystem::path> estructura_archivos;
    
    estructura_directorios.push_back(directorio_base / nombre_carpeta_raiz);
    
    // Usaremos una pila para mantener la ruta actual según el nivel de indentación
    std::stack<std::filesystem::path> pila_rutas;
    std::stack<int> pila_niveles;
    
    pila_rutas.push(directorio_base / nombre_carpeta_raiz);
    pila_niveles.push(0);
    
    for (const std::string& linea_val : lineas_arbol)
    {
        std::string linea_limpia = linea_val;
        
        // Eliminar retornos de carro y saltos de línea
        if (!linea_limpia.empty() && linea_limpia.back() == '\r')
        {
            linea_limpia.pop_back();
        }
        if (!linea_limpia.empty() && linea_limpia.back() == '\n')
        {
            linea_limpia.pop_back();
        }
        
        // Saltar líneas vacías
        if (linea_limpia.empty())
        {
            continue;
        }
        
        // Detectar si es línea de árbol (contiene caracteres de árbol)
        bool tiene_patron_arbol = (linea_limpia.find('├') != std::string::npos) ||
                                  (linea_limpia.find('└') != std::string::npos) ||
                                  (linea_limpia.find("──") != std::string::npos) ||
                                  (linea_limpia.find('│') != std::string::npos);
        
        if (tiene_patron_arbol)
        {
            // Calcular nivel por indentación (contar prefijo de árbol)
            int nivel_actual = 0;
            int caracteres_arbol = 0;
            
            // Contar caracteres de árbol antes del nombre
            for (char caracter : linea_limpia)
            {
                if (caracter == '├' || caracter == '└' || caracter == '─' || caracter == '│' || caracter == ' ')
                {
                    caracteres_arbol++;
                }
                else
                {
                    break;
                }
            }
            
            // Calcular nivel basado en caracteres de árbol
            nivel_actual = caracteres_arbol / 4;
            
            // Extraer nombre del elemento (eliminar caracteres de árbol)
            std::string nombre_elemento = "";
            bool en_nombre = false;
            
            for (char caracter : linea_limpia)
            {
                if (!en_nombre && caracter != '├' && caracter != '└' && caracter != '─' && caracter != '│' && caracter != ' ')
                {
                    en_nombre = true;
                }
                
                if (en_nombre)
                {
                    nombre_elemento += caracter;
                }
            }
            
            // Eliminar espacios al inicio y final
            size_t inicio = nombre_elemento.find_first_not_of(" \t");
            size_t fin = nombre_elemento.find_last_not_of(" \t");
            
            if (inicio != std::string::npos && fin != std::string::npos)
            {
                nombre_elemento = nombre_elemento.substr(inicio, fin - inicio + 1);
            }
            else
            {
                nombre_elemento = "";
            }
            
            if (nombre_elemento.empty())
            {
                continue;
            }
            
            // Si el nivel actual es menor o igual al anterior, retroceder en la pila
            while (pila_niveles.size() > 1 && pila_niveles.top() >= nivel_actual)
            {
                pila_rutas.pop();
                pila_niveles.pop();
            }
            
            // Obtener la ruta padre actual
            std::filesystem::path ruta_padre = pila_rutas.top();
            
            // Construir la ruta completa
            std::filesystem::path ruta_completa = ruta_padre / nombre_elemento;
            
            // Determinar si es directorio o archivo
            if (es_archivo(nombre_elemento))
            {
                // Es un archivo
                estructura_archivos.push_back(ruta_completa);
            }
            else
            {
                // Es un directorio
                estructura_directorios.push_back(ruta_completa);
                pila_rutas.push(ruta_completa);
                pila_niveles.push(nivel_actual);
            }
        }
    }
    
    return std::make_pair(estructura_directorios, estructura_archivos);
}

// Extrae y procesa los contenidos de los archivos del documento
std::unordered_map<std::string, std::string> extraer_contenidos_archivos(const std::vector<std::string>& lineas_documento)
{
    std::unordered_map<std::string, std::string> contenidos_archivos;
    std::string archivo_actual;
    std::vector<std::string> contenido_actual;
  
    bool en_contenido_archivo = false;
    bool primera_linea_contenido = true;
    
    for (const std::string& linea_val : lineas_documento)
    {
        std::string linea_limpia = linea_val;
        
        // Eliminar retornos de carro y saltos de línea
        if (!linea_limpia.empty() && linea_limpia.back() == '\r')
        {
            linea_limpia.pop_back();
        }
        if (!linea_limpia.empty() && linea_limpia.back() == '\n')
        {
            linea_limpia.pop_back();
        }
        
        // Detectar separadores
        if (linea_limpia.find("------------------------------------") != std::string::npos)
        {
            // Guardar archivo anterior si existe
            if (!archivo_actual.empty())
            {
                // Unir contenido y guardar (puede estar vacío)
                std::string contenido_final;
                
                for (size_t i = 0; i < contenido_actual.size(); ++i)
                {
                    contenido_final += contenido_actual[i];
                    
                    if (i < contenido_actual.size() - 1)
                    {
                        contenido_final += "\n";
                    }
                }
                
                // Eliminar espacios finales
                size_t fin = contenido_final.find_last_not_of(" \t\n\r");
                
                if (fin != std::string::npos)
                {
                    contenido_final = contenido_final.substr(0, fin + 1);
                }
                
                contenidos_archivos[archivo_actual] = contenido_final;
                archivo_actual.clear();
                contenido_actual.clear();
                en_contenido_archivo = false;
                primera_linea_contenido = true;
            }
            
            continue;
        }
        
        // Detectar nuevo archivo (línea que contiene ruta de archivo con extensión)
        if (archivo_actual.empty() && 
            linea_limpia.find('.') != std::string::npos && 
            linea_limpia.find('├') == std::string::npos && 
            linea_limpia.find('└') == std::string::npos && 
            linea_limpia.find('│') == std::string::npos && 
            linea_limpia.find('─') == std::string::npos &&
            linea_limpia.find_first_not_of(" \t") == 0)
        {
            archivo_actual = linea_limpia;
            contenido_actual.clear();
            en_contenido_archivo = true;
            primera_linea_contenido = true;
          
            continue;
        }
        
        // Agregar contenido al archivo actual si estamos en sección de contenido
        if (en_contenido_archivo && !archivo_actual.empty())
        {
            // Si es la primera línea de contenido y está vacía, saltarla
            if (primera_linea_contenido && linea_limpia.empty())
            {
                continue;
            }
            
            // Marcar que ya pasamos la primera línea de contenido
            primera_linea_contenido = false;
            contenido_actual.push_back(linea_limpia);
        }
    }
    
    // Guardar último archivo procesado
    if (!archivo_actual.empty())
    {
        std::string contenido_final;
        
        for (size_t i = 0; i < contenido_actual.size(); ++i)
        {
            contenido_final += contenido_actual[i];
            
            if (i < contenido_actual.size() - 1)
            {
                contenido_final += "\n";
            }
        }
        
        // Eliminar espacios finales
        size_t fin = contenido_final.find_last_not_of(" \t\n\r");
        
        if (fin != std::string::npos)
        {
            contenido_final = contenido_final.substr(0, fin + 1);
        }
        
        contenidos_archivos[archivo_actual] = contenido_final;
    }
    
    return contenidos_archivos;
}

// Procesa el archivo de estructura y genera los directorios y archivos
bool procesar_archivo_estructura(const std::filesystem::path& ruta_archivo)
{
    // Obtener directorio base donde se encuentra el archivo TXT
    std::filesystem::path directorio_base = ruta_archivo.parent_path();
    
    // Leer todo el contenido del archivo
    std::ifstream archivo_val(ruta_archivo);
    
    if (!archivo_val.is_open())
    {
        std::cout << "Cannot open file\n";
      
        return false;
    }
    
    std::vector<std::string> lineas;
    std::string linea_temp;
    
    while (std::getline(archivo_val, linea_temp))
    {
        lineas.push_back(linea_temp);
    }
    
    archivo_val.close();
    
    if (lineas.empty())
    {
        std::cout << "Empty file\n";
      
        return false;
    }
    
    // Extraer nombre de carpeta raiz (primera línea)
    std::string nombre_carpeta_raiz = lineas[0];
    
    // Eliminar espacios al inicio y final
    size_t inicio = nombre_carpeta_raiz.find_first_not_of(" \t\r\n");
    size_t fin = nombre_carpeta_raiz.find_last_not_of(" \t\r\n");
    
    if (inicio != std::string::npos && fin != std::string::npos)
    {
        nombre_carpeta_raiz = nombre_carpeta_raiz.substr(inicio, fin - inicio + 1);
    }
    else
    {
        nombre_carpeta_raiz = "";
    }
    
    if (nombre_carpeta_raiz.empty())
    {
        std::cout << "No root folder name found\n";
      
        return false;
    }
    
    // Separar secciones
    std::vector<std::string> lineas_arbol;
    std::vector<std::string> lineas_contenidos;
  
    bool en_contenidos = false;
    
    for (size_t i = 1; i < lineas.size(); ++i)
    {
        const std::string& linea_val = lineas[i];
        std::string linea_limpia = linea_val;
        
        // Eliminar retornos de carro
        if (!linea_limpia.empty() && linea_limpia.back() == '\r')
        {
            linea_limpia.pop_back();
        }
        
        if (linea_limpia.find("------------------------------------") != std::string::npos)
        {
            en_contenidos = true;
        }
        
        if (en_contenidos)
        {
            lineas_contenidos.push_back(linea_val);
        }
        else
        {
            lineas_arbol.push_back(linea_val);
        }
    }
    
    // Procesar estructura
    auto [directorios, archivos] = procesar_estructura_directorios(lineas_arbol, nombre_carpeta_raiz, directorio_base);
    auto contenidos = extraer_contenidos_archivos(lineas_contenidos);
    
    // Crear directorios
    for (const auto& directorio_val : directorios)
    {
        std::filesystem::create_directories(directorio_val);
    }
    
    // Crear archivos con sus contenidos
    for (const auto& archivo_ruta : archivos)
    {
        std::string nombre_archivo = archivo_ruta.filename().string();
        
        // Buscar contenido por nombre de archivo
        std::string contenido_encontrado;
        bool contenido_encontrado_flag = false;
        
        for (const auto& [clave_archivo, contenido_val] : contenidos)
        {
            if (clave_archivo.length() >= nombre_archivo.length() &&
                clave_archivo.compare(clave_archivo.length() - nombre_archivo.length(), nombre_archivo.length(), nombre_archivo) == 0)
            {
                contenido_encontrado = contenido_val;
                contenido_encontrado_flag = true;
              
                break;
            }
        }
        
        if (contenido_encontrado_flag)
        {
            procesar_contenido_archivo(contenido_encontrado, archivo_ruta);
        }
        else
        {
            // Crear archivo vacío si no se encuentra contenido
            procesar_contenido_archivo("", archivo_ruta);
        }
    }
    
    // Mostrar estadísticas
    std::cout << "Output: " << directorios.size() << " directories, " << archivos.size() << " files\n";
  
    return true;
}

// BUCLE PRINCIPAL
int main()
{
    while (true)
    {
        // Solicitar directorio del archivo TXT
        std::cout << "TXT file directory: ";
      
        std::string directorio_entrada;
      
        std::getline(std::cin, directorio_entrada);
        
        // Eliminar comillas si existen
        if (!directorio_entrada.empty() && (directorio_entrada.front() == '"' || directorio_entrada.front() == '\''))
        {
            directorio_entrada = directorio_entrada.substr(1);
        }
        if (!directorio_entrada.empty() && (directorio_entrada.back() == '"' || directorio_entrada.back() == '\''))
        {
            directorio_entrada.pop_back();
        }
        
        // Verificar que el directorio existe
        if (!std::filesystem::exists(directorio_entrada))
        {
            std::cout << "Wrong directory\n";
          
            continue;
        }
        
        // Ejecutar procesamiento de estructura
        bool resultado_procesamiento = procesar_archivo_estructura(directorio_entrada);
        
        // Si el procesamiento falla, mostrar mensaje
        if (!resultado_procesamiento)
        {
            std::cout << "Processing failed\n";
        }
        
        std::cout << "\n";
    }
    
    return 0;
}
