if not exist ".\venv\" python -m venv venv
call .\venv\Scripts\activate
pip install wgrd-cons-tools
modding_suite.exe
