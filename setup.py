from cx_Freeze import setup, Executable

setup(name="Directory generator", executables=[Executable("Directory generator script.py")], options={"build_exe": {"excludes": ["tkinter"]}})