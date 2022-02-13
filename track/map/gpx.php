<?php 
require __DIR__ . '/../vendor/autoload.php';
header('Content-Type: text/xml');
//header('Content-Type: text/plain');

$credentials = json_decode(file_get_contents("../secrets/mongodb.json"));
$time = gmdate("Y-m-d\TH:i:s\Z");
$http_origin = $_SERVER['HTTP_ORIGIN'];
$author = "Samantha Finnigan";

$gpx_header = <<<XML
<?xml version="1.0" encoding="UTF-8" standalone="no"?> 

<gpx xmlns="http://www.topografix.com/GPX/1/1" xmlns:gpxx="http://www.garmin.com/xmlschemas/GpxExtensions/v3" 
     xmlns:gpxtpx="http://www.garmin.com/xmlschemas/TrackPointExtension/v1" creator="Oregon 400t" version="1.1" 
     xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" 
     xsi:schemaLocation="http://www.topografix.com/GPX/1/1 http://www.topografix.com/GPX/1/1/gpx.xsd">

<metadata>
    <link href="$http_origin">
        <text>$author</text>
    </link>
    <time>$time</time>
</metadata>

<trk>
<trkseg>

XML;

/**
 * Main function
 */
function main() {
    global $credentials;

	// Update date in range to from parameter
	if(! ($_GET['from'] && $_GET['to'])) {
		http_response_code(400);
		throw new ErrorException("Provide <from> and <to> parameters to GET");
	}

	$bound_from = strval(intval($_GET['from']) * 1000000);
	$bound_to   = strval(intval($_GET['to']) * 1000000);

	// Get the client
	$client = new MongoDB\Client($credentials->uri);
    $db = $client->track;
    $co = $db->gps;

    if(is_null($co)) {
        http_response_code(400);
        throw new ErrorException("Collection not found. Maybe there was no data yet?");
    }

    $cursor = $co->find( [
            'utctime' => [
                "\$gte" => 20220207000000, 
                "\$lt"  => 20220208000000
    ]]);
    
    // Do this at the end to allow errors to interrupt
	write_header();
    write_rows($cursor);
}

function write_header() {
	global $gpx_header;
	print($gpx_header);
}

function write_rows($values) {
	foreach($values as $row) {
        if(!is_null($row['segmentstart']))
            print("</trkseg><trkseg>\n");

		printf("<trkpt lat=\"%s\" lon=\"%s\"><ele>%s</ele><time>%s</time></trkpt>\n", 
                $row['latitude'], 
                $row['longitude'], 
                $row['altitude'], 
                $row['utctime']
        );
	}
}

main();

?>
</trkseg>
</trk>
</gpx>
