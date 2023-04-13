import os
import glob
import subprocess

# Check if lupdate is installed
try:
    subprocess.check_output(['lupdate', '-version'])
except FileNotFoundError:
    try:
        subprocess.check_output(['lupdate.exe', '-version'])
    except FileNotFoundError:
        print("lupdate is not installed. Please install it and try again.")
        exit(1)

# Set the working directory to the parent directory of the script
script_dir = os.path.dirname(os.path.abspath(__file__))
os.chdir(os.path.join(script_dir, '..'))

# Run lupdate for each translation file
src_files = glob.glob(os.path.join(script_dir, '..', 'src', '*.cpp'))
plugin_files = glob.glob(os.path.join(script_dir, '..', 'plugins', '**', '*.cpp'))
plugin_header_files = glob.glob(os.path.join(script_dir, '..', 'plugins', '*.h'))
speechtotext_files = glob.glob(os.path.join(script_dir, '..', 'speechtotext', '**', '*.cpp'))
ui_files = glob.glob(os.path.join(script_dir, '..', 'ui', '*.ui'))

for ts_file in glob.glob(os.path.join(script_dir, '..', 'translations', 'VoiceAssistant_*.ts')):
    subprocess.run(['lupdate' if os.name == 'posix' else 'lupdate.exe', *src_files, *plugin_files, *plugin_header_files, *speechtotext_files, *ui_files, '-ts', ts_file, '-noobsolete'])
