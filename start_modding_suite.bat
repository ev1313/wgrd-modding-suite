if not exist ".\venv\" python -m venv venv
call .\venv\Scripts\activate
pip install wgrd-cons-parsers --upgrade
pip install wgrd-cons-tools --upgrade
set PYTHONPATH=.\venv\Lib\site-packages
modding_suite.exe
