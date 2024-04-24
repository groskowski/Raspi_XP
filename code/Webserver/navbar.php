<?php
    $navItems = [
        'dash' => ['url' => 'index.php', 'label' => 'Dashboard', 'icon' => 'fa-home', 'visible' => true],
        'map' => ['url' => 'map.php', 'label' => 'Map', 'icon' => 'fa-map', 'visible' => true],
        'dev' => ['url' => 'dev.php', 'label' => 'Dev', 'icon' => 'fa-book', 'visible' => true],
    ];

    $navItems = array_map(function ($item) {
        $item['url'] = $item['url'] ?? '#';
        $item['label'] = $item['label'] ?? 'No label';
        $item['icon'] = $item['icon'] ?? 'fa-icon';
        $item['visible'] = $item['visible'] ?? true;
        return $item;
    }, $navItems);

    $listContent = $listContent ?? ""
?>
<style>
    .navigation ul li:last-child {
        margin-left: auto;
    }

    .navbuttons {
        display: flex;  
    }

    .hamburger-menu {
        display: none;
        font-size: 24px;
        background: none;
        border: none;
        color: white;
    }

    .hamburger-menu:hover {
        cursor:pointer;
    }

    @media (max-width: 768px) {
        .navbuttons {
            flex-direction: column;
            width: 100%;
            display: none;
        }

        .navbuttons li {
            width: 100%;
        }

        .navigation {
            flex-direction: column;
        }

        .nav-logo {
            margin-top: 10px;
        }
    }

    @media (max-width: 768px) {
        .hamburger-menu {
            display: block;
        }
    }
</style>
<div id="overlay" class="overlay">
    <div class="loading-bar-container"><span id="loadingText" class="loading-text">Waiting for response</span>
        <div id="loadingBar" class="loading-bar"></div>
    </div>
</div>
<header>
    <button class="hamburger-menu" onclick="toggleMenu()">☰</button>
    <nav class="navigation">
        <ul class="navbuttons">
            <?php foreach ($navItems as $item) {
                if ($item['visible']) { ?>
                    <li><a href="<?php echo $item['url']; ?>" class="navbar-buttons"><i class="fa <?php echo $item['icon']; ?>"></i> <?php echo $item['label']; ?></a></li>
                <?php }
            } ?>
            <?php echo $listContent?>
            <li style="width:auto;"><img src="img/iit.png" alt="IIT Logo" style="width: 150px; float: right;"></li>
        </ul>
    </nav>
</header>
<script>
    function toggleMenu() {
        var nav = document.querySelector('.navbuttons');
        if (nav.style.display === 'block') {
            nav.style.display = 'none';
        } else {
            nav.style.display = 'block';
        }
    }
    function logMessageToServer(message, type) {
        var xhr = new XMLHttpRequest();
        xhr.open("POST", "php/log.php", true);
        xhr.setRequestHeader("Content-type", "application/x-www-form-urlencoded");
        xhr.onreadystatechange = function() {
            if (xhr.readyState === 4 && xhr.status === 200) {
                //console.log(xhr.responseText);
            }
        };
        var data = "message=" + encodeURIComponent(message) + "&type=" + encodeURIComponent(type);
        xhr.send(data);
    }
    function logInfo(message){
        logMessageToServer(message, "info");
    }
    function logWarning(message){
        logMessageToServer(message, "warning");
    }
    function logError(message){
        logMessageToServer(message, "error");
    }
</script>