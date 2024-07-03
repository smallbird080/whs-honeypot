#!/usr/bin/php
<?php
// 시작 전에 데이터베이스 연결 또는 필요한 설정을 로드합니다.
// 예: require 'db.php';

session_start();

if ($_SERVER["REQUEST_METHOD"] == "POST") {
    $username = $_POST['username'];
    $password = $_POST['password'];

    if ($username == 'admin' || $username == 'Admin' || $username == 'Manager' || $username == 'manager') {
        // 검증이 성공하면, 사용자 정보를 세션에 저장합니다.
        $_SESSION['loggedin'] = true;
        $_SESSION['username'] = $username;

        // 사용자를 메인 페이지로 리디렉션합니다.
        header('Location: index.php');
        exit;
    } else {
        // 검증이 실패하면, 에러 메시지를 표시합니다.
        $error = "Invalid username or password.";
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

    label {
        display: block;
        margin-top: 10px;
    }

    input {
        padding: 5px;
        margin-top: 5px;
    }

    button {
        padding: 5px 10px;
        margin-top: 10px;
        background-color: #007bff;
        color: white;
        border: none;
        cursor: pointer;
    }
<html>
<head>
    <title>Login</title>
</head>
<body>
    <h2>Login</h2>
    <?php if (!empty($error)): ?>
        <p style="color: red;"><?php echo $error; ?></p>
    <?php endif; ?>
    <form action="authLogin.php" method="post">
        <label for="username">Username:</label>
        <input type="text" id="username" name="username"><br>
        <label for="password">Password:</label>
        <input type="password" id="password" name="password"><br>
        <button type="submit">Login</button>
    </form>
</body>
</html>