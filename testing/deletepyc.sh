#!/bin/bash

find ./ -name "*.pyc" -exec rm {} \; 
find ./ -name "*.py~" -exec rm {} \; 

