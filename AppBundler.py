import sys
from pathlib import Path
from base64 import b64encode
import PyInstaller.__main__
import os
import os.path
import shutil


RUN_PREFIX = "./" if (os.name != "nt") else ""


if len(sys.argv) < 3:
        print("USAGE: AppBundler <PATH/TO/TARGET/DIR> <PATH/TO/APP/FROM/WITHIN> [ARGS...]")
        exit()

sys.argv[2] = RUN_PREFIX + sys.argv[2]

shutil.make_archive("_TEMP", "tar", sys.argv[1])
target_file_data = ""
with open("_TEMP.tar", "rb") as target:
        target_file_data = b64encode(target.read()).decode("ascii")
os.remove("_TEMP.tar")

with open("OUT.py", "w") as out:
        out.write(
                "from tempfile import gettempdir\n"
                "import os\n"
                "from base64 import b64decode\n"
                "from shutil import unpack_archive\n"
                "import subprocess\n"
                "import sys\n"

                "DATA = '''" + target_file_data + "'''\n"

                "os.chdir(gettempdir())\n"
                "with open('TEMP.tar', 'wb') as f:\n"
                "\tf.write(b64decode(DATA))\n"
                "unpack_archive('TEMP.tar')\n"
                "os.remove('TEMP.tar')\n"
                "if os.name != 'nt': os.chmod('" + sys.argv[2] + "', 0o777)\n"
                "subprocess.Popen(" + repr(sys.argv[2:]) + ")\n"
                
                "sys.exit()\n"
        )

PyInstaller.__main__.run([
        "OUT.py",
        "--onefile",
        #"--windowed", # UNCOMMENT TO STOP TERMINAL FROM APPEARING ON WINDOWS
        "--noupx",
        "--distpath", ".",
        "--clean"
])

if os.path.isdir("build"):
        shutil.rmtree("build")
if os.path.isfile("OUT.spec"):
        os.remove("OUT.spec")

os.remove("OUT.py")