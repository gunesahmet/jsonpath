--TEST--
Testing >, <, >= <= operators
--SKIPIF--
<?php if (!extension_loaded("jsonpath")) print "skip"; ?>
--FILE--
<?php

$jsonPath = new JsonPath();

$obj = array (
  'test' => array(
      array (
        'id' => 1,
        'val' => 12,
        'val_str' => "table"
      ),
   )
);

var_dump($jsonPath->find($obj, '$.test[?(@.val <= 11)]'));
--EXPECT--
bool(false)
