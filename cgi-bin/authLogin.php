<?php
session_start();
if ($_SERVER['REQUEST_METHOD'] == 'POST') {
    $username = $_POST['username'];
    $password = $_POST['password'];

    if ($username == 'admin' || $username == 'Admin' || $username == 'Manager' || $username == 'manager') {
        $_SESSION['loggedin'] = true;
        $_SESSION['username'] = $username;
        header('Location: index.php');
        exit;
    } else {
        $error = 'Incorrect username or password!';
    }
}
?>
<!DOCTYPE html>
<style>
    body {
        font-family: Arial, sans-serif;
        background-color: #f2f2f2;
    }

    h2 {
        color: #333;
    }

    p {
        color: #666;
    }
</style>
<html>
<head>
    <title>Login</title>
</head>
<body>
    <h2>Login</h2>
    <form action="login.php" method="post">
        Username: <input type="text" name="username"><br>
        Password: <input type="password" name="password"><br>
        <input type="submit" value="Login">
    </form>
    <?php if (isset($error)): ?>
        <p><?php echo $error; ?></p>
    <?php endif; ?>
</body>
</html>