import sys
from pathlib import Path
from tempfile import gettempdir


if len(sys.argv) < 3:
        print("USAGE: AppBundler <TARGET>.zip <PATH/TO/APP/WITHIN> [ARGS...]")
        exit()


target_file_data = ""
with open(sys.argv[1], "r") as target:
        target_file_data = target.read()

target_file_name = Path(sys.argv[1]).stem

with open("_TEMP.py", "w") as out:
        out.write(
                "import os\n" +
                "from zipfile import ZipFile\n" +
                "DATA = \"" + target_file_data + "\"\n" +
                #"os.chdir(\"" + gettempdir() + "\")\n" +
                "with open(\"TEMP.zip\", \"w\") as f:\n" +
                "\tf.write(DATA)\n" +
                "os.mkdir(\"" + target_file_name + "\")\n" +
                "with ZipFile(\"TEMP.zip\", \"w\") as z:\n" +
                "\tz.extractall(\"" + target_file_name + "\")\n" +
                "os.system(\"" + " ".join(sys.argv[3:]) + "\")\n" +
                "exit()"
        )

# TODO: Use Pyinstaller on it ^