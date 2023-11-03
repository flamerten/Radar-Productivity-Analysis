# Venv Instructions

## Create the Virtual Env

```bash
$ pip install virtualenv
$ virtualenv --version

$ cd project_folder
$ python -m virtualenv venv

#Activate with Linux
$ source venv/bin/activate

#Activate with Windows
> venv\Scripts\activate

#Get out of env
$ deactivate
```

## Requirements
```bash
$ pip freeze > requirements.txt #save list of packages in current env
$ pip install -r requirements.txt #install based on a requirements.txt file
```