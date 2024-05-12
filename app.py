from flask import Flask, render_template, redirect, request
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
    dev = True
    if dev:
        return render_template('dev.html')
    else:
        num_vars = request.form.get('num_vars')
        minterms = request.form.get('minterms')
        dont_cares = request.form.get('dont_cares')

        if not all([num_vars, minterms, dont_cares]):
            return "Invalid input", 400

        try:
            minterms_list = [int(num) for num in minterms.split(',')]
            dont_cares_list = [int(num) for num in dont_cares.split(',')]
        except ValueError:
            return "Invalid number format", 400

        minterms_str = ' '.join(map(str, minterms_list))
        dont_cares_str = ' '.join(map(str, dont_cares_list))

        command = f"python boolean.py {num_vars} {minterms_str} {dont_cares_str}"

        process = subprocess.Popen(command, shell=True, stdout=subprocess.PIPE)
        output, _ = process.communicate()

        if process.returncode != 0:
            return "Error executing script", 500

        output = output.decode().strip()
        return render_template('bool.html', output=output)

@app.errorhandler(404)
def page_not_found(e):
    return render_template('404.html'), 404

@app.errorhandler(500)
def internal_server_error(e):
    return render_template('500.html'), 500

if __name__ == '__main__':
    ip = "127.0.0.1"
    debug = False
    app.run(host=ip,debug=debug)
    