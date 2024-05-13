from flask import Flask, render_template, redirect, request, flash
import subprocess, logging

app = Flask(__name__)
logging.basicConfig(filename='app.log', level=logging.INFO)

@app.route('/')
def home():
    return render_template('main.html')

@app.route('/calc')
def calculator():
    return render_template('calc.html')

@app.route('/bool',methods=['GET','POST'])
def boolean():
    error_message = None
    if request.method == 'POST':
        num_vars = request.form['num_vars']
        minterms = request.form['minterms']
        dont_cares = request.form['dont_cares']

        if not all(char.isnumeric() or char.isspace() for char in minterms + dont_cares):
            error_message = 'Invalid input, please try again. (only numbers and spaces allowed)'
            return render_template('bool.html', error_message=error_message)

        try:
            minterms_str = " ".join(minterms.split())
            dont_cares_str = " ".join(dont_cares.split())
        except:
            error_message = 'Invalid input, please try again. (Hint: Use spaces to separate numbers)'
            return render_template('bool.html', error_message=error_message)

        command = f"python3 boolean.py {num_vars} "+'"'+f"{minterms_str}"+'" "'+ f"{dont_cares_str}"+'"'

        process = subprocess.Popen(command, shell=True, stdout=subprocess.PIPE)
        output, _ = process.communicate()

        if process.returncode != 0:
            error_message = 'Invalid input, please try again.'
            return render_template('bool.html', error_message=error_message)

        output = output.decode()
        return render_template('bool.html', output=output, error_message=error_message)
    else:
        return render_template('bool.html', error_message=error_message)

@app.errorhandler(404)
def page_not_found(e):
    return render_template('404.html'), 404

@app.errorhandler(500)
def internal_server_error(e):
    return render_template('500.html'), 500

if __name__ == '__main__':
    ip = "127.0.0.1"
    debug = True
    app.run(host=ip,debug=debug)
    