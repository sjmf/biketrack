<?php
require __DIR__ . '/vendor/autoload.php';

$credentials = json_decode(file_get_contents("secrets/mongodb.json"));
$client = new MongoDB\Client($credentials->uri);

if(php_sapi_name()==="cli") // Running from CLI?
    return;

function main() {
    // Handle Tracker POSTS to this API
    if ($_SERVER['REQUEST_METHOD'] === 'POST' && $_SERVER['HTTP_USER_AGENT'] === "FONA") {
        return handle_tracker_post();
    } 

    // GET access from a browser?
    if(preg_match('/Mozilla/i',$_SERVER['HTTP_USER_AGENT'])) {
        echo "<pre>";
    }
    print_r(file_get_contents("loc.htm"));
    print_r("\n");
}

function handle_tracker_post() {
    file_put_contents("loc.htm", file_get_contents("php://input"));
    $data = explode(',', $_POST['gps']);
    array_pop($data);
    array_push($data, $_POST['vbat'], $_POST['vpc']);
    
    // Data type conversion for reference:
    // Cols: 1-2 (int) 3-8 (float) 9 (int) 10 (emptystring) 11-13 (float) 14 (empty) 15-16 (int) 17-18 (empty) 19 (int) 20 (empty) 21-22 (int)
    write_to_mongo($data);
    echo 'k'; // Due to a bug in Adafruit_FONA.cpp (line 2277) we must return a body of >1 byte long.
}

function write_to_mongo($data) {
    global $client;

    $db = $client->track;
    $co = $db->gps;

    if(is_null($co)) {
        $db->createCollection("gps");
    }
    $document = array(
        "utctime"         => intval($data[2]),
        "latitude"        => floatval($data[3]),
        "longitude"       => floatval($data[4]),
        "altitude"        => floatval($data[5]),
        "speed"           => floatval($data[6]),
        "course"          => floatval($data[7]),
        "HDOP"            => floatval($data[10]),
        "PDOP"            => floatval($data[11]),
        "VDOP"            => floatval($data[12]),
        "view_satellites" => intval($data[14]),
        "used_satellites" => intval($data[15]),
        "HPA"             => intval($data[18]),
        "vBat"            => intval($data[20]),
        "vPc"             => intval($data[21])
    );
    $co->insertOne($document);
}

try {
    main();
}
catch (Exception $e) {
    http_response_code(500);
    print_r($e->getMessage());
}
?>

