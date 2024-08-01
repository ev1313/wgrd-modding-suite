if not exist ".\venv\" python -m venv venv
call .\venv\Scripts\activate
pip install wgrd-cons-tools
set PYTHONPATH=.\venv\Lib\site-packages
modding_suite.exe
