import sys
from pathlib import Path
from base64 import b64encode
from tempfile import gettempdir
import PyInstaller.__main__
import os


if len(sys.argv) < 3:
        print("USAGE: AppBundler <TARGET>.zip <PATH/TO/APP/WITHIN> [ARGS...]")
        exit()


target_file_data = ""
with open(sys.argv[1], "rb") as target:
        target_file_data = b64encode(target.read()).decode("ascii")

target_file_name = Path(sys.argv[1]).stem

with open("_TEMP.py", "w") as out:
        out.write(
                "import os\n" +
                "from base64 import b64decode\n" +
                "from zipfile import ZipFile\n" +
                "import subprocess\n" +
                "DATA = '''" + target_file_data + "'''\n" +
                "os.chdir(\"" + gettempdir() + "\")\n" + # COMMENTED OUT FOR TESTING
                "with open(\"TEMP.zip\", \"wb\") as f:\n" +
                "\tf.write(b64decode(DATA))\n" +
                "os.makedirs(\"" + target_file_name + "\", exist_ok=True)\n" +
                "with ZipFile(\"TEMP.zip\", \"r\") as z:\n" +
                "\tz.extractall(\"" + target_file_name + "\")\n" +
                "os.remove(\"TEMP.zip\")\n" +
                "os.chdir(\"" + target_file_name + "\")\n" +
                "subprocess.Popen(" + repr(sys.argv[2:]) + ", creationflags=0x8, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)\n" +
                "exit()"
        )

PyInstaller.__main__.run([
        "_TEMP.py",
        "--onefile"
])

os.remove("_TEMP.py")