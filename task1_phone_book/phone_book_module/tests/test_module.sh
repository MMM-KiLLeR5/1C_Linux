#!/bin/sh

# Add
echo "Test add"
echo "add Mike Abbot 18 +7999887766 mike.abbot@gmail.com" > /dev/mipt_pb
cat /dev/mipt_pb # User created.
echo "--------"

# Get
echo "Test get"
echo "get Abbot" > /dev/mipt_pb
cat /dev/mipt_pb
echo "--------"

# Delete
echo "Test delete"
echo "del Abbot" > /dev/mipt_pb
cat /dev/mipt_pb
echo "--------"
