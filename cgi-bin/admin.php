<?php
session_start();
if (!isset($_SESSION['loggedin'])) {
    header('Location: login.php');
    exit;
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

    a {
        color: #007bff;
        text-decoration: none;
    }
</style>
<html>
<head>
    <title>Admin Page</title>
</head>
<body>
    <h1>Welcome to the Admin Page, <?php echo $_SESSION['username']; ?>!</h1>
    <a href="logout.php">Logout</a>
</body>
</html>