<?php
session_start();
if (!isset($_SESSION['username'])) {
    echo '<a href="login.php">Go to Login Page</a>';
}
?>
<!DOCTYPE html>
<style>
    body {
        font-family: Arial, sans-serif;
        background-color: #f2f2f2;
    }

    h1 {
        color: #333;
    }

    p {
        color: #666;
    }

    a {
        color: #007bff;
        text-decoration: none;
    }
</style>
<html>
<head>
    <title>Main Page</title>
</head>
<body>
    <h1>Welcome <?php echo $_SESSION['username']; ?>!</h1>
    <p>This is the main page.</p>
    <a href="logout.php">Logout</a>
</body>
</html>