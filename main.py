# ------------------------------------------------------------------------------------------------ #

import subprocess

# ------------------------------------------------------------------------------------------------ #

'''

[python.modules.dp-cvep-speller]

custom_entry_point = 'main'

ip = '127.0.0.1'

port = 8084

retry_after_s = 3.0

max_connect_retries = 10

'''

# ------------------------------------------------------------------------------------------------ #

import os

def main():

    path = r'D:\Users\alper\msys64\ucrt64\bin'
    
    cwd = os.path.dirname(os.path.abspath(__file__))

    subprocess.run('taskkill /f /im main.exe 2>NUL', shell = True)

    subprocess.run(f'set PATH={path};%PATH% && gcc main.c -O3 -l lsl -l ws2_32 -l SDL3 -l SDL3_ttf -l winmm -l avrt -o main.exe', shell = True, cwd = cwd)

    subprocess.run(f'set PATH={path};%PATH% && start "" "main.exe"', shell = True, cwd = cwd)

# ------------------------------------------------------------------------------------------------ #

if __name__ == '__main__':

    main()

# ------------------------------------------------------------------------------------------------ #