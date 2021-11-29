## OSU CS 344 Assignment 5 - One-Time Pads

### Compilation

- To compile, use the compileall script. This script creates executables for:
    - keygen
    - enc_client
    - enc_server
    - dec_client
    - dec_server

- To execute script, run `./compileall`
- If a permissions error is encountered, run `chmod u+x ./compileall` before executing script

### To run test script

- Run `./p5testscript RANDOM_PORT1 RANDOM_PORT2 > mytestresults 2>&1`
- If a permissions error is encountered, run `chmod u+x ./p5testscript`

#### Notes

- Make sure to look in header files for function comments