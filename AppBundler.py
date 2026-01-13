import sys
from pathlib import Path
from base64 import b64encode
import PyInstaller.__main__
import os
import os.path
import shutil


RUN_PREFIX = "./" if (os.name != "nt") else ""


if len(sys.argv) < 3:
        print("USAGE: AppBundler <TARGET>.zip <PATH/TO/APP/WITHIN> [ARGS...]")
        exit()

sys.argv[2] = RUN_PREFIX + sys.argv[2]

target_file_data = ""
with open(sys.argv[1], "rb") as target:
        target_file_data = b64encode(target.read()).decode("ascii")

target_file_name = Path(sys.argv[1]).stem

with open("OUT.py", "w") as out:
        out.write(
                "from tempfile import gettempdir\n"
                "import os\n"
                "from base64 import b64decode\n"
                "from zipfile import ZipFile\n"
                "import subprocess\n"
                "import sys\n"
                "DATA = '''" + target_file_data + "'''\n"
                "os.chdir(gettempdir())\n"
                "with open('TEMP.zip', 'wb') as f:\n"
                "\tf.write(b64decode(DATA))\n"
                "os.makedirs('" + target_file_name + "', exist_ok=True)\n"
                "with ZipFile('TEMP.zip', 'r') as z:\n"
                "\tz.extractall('" + target_file_name + "')\n"
                "os.remove('TEMP.zip')\n"
                "os.chdir('" + target_file_name + "')\n"
                "if os.name != 'nt': os.chmod('" + sys.argv[2] + "', 0o777)\n"
                "subprocess.Popen(" + repr(sys.argv[2:]) + ")\n"
                "sys.exit()\n"
        )

PyInstaller.__main__.run([
        "OUT.py",
        "--onefile",
        "--windowed",
        "--noupx",
        "--distpath", ".",
        "--clean"
])

if os.path.isdir("build"):
        shutil.rmtree("build")
if os.path.isfile("OUT.spec"):
        os.remove("OUT.spec")

os.remove("OUT.py")