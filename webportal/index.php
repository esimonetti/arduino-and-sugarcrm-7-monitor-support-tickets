<?php
require_once('vendor/autoload.php');

// SugarCRM Details
$sugar_url = 'https://your-sugarcrm-url.com/rest/v10/';
$sugar_user = 'your_user';
$sugar_password = 'your_password';
// End  SugarCRM Details

$sugar = new \Spinegar\Sugar7Wrapper\Rest();
$sugar->setUrl($sugar_url)->setUsername($sugar_user)->setPassword($sugar_password)->connect();

// count new cases records
$cases = $sugar->countRecords('Cases',
        array(
                'filter' => array(
                        array('status' => 'New'),
                ),
        )
);

if(!empty($cases))
{
        echo json_encode(
                array(
                        'open_cases' => $cases['record_count']
                )
        );
}
