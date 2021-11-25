/*
Usage: dec_client <ciphertext> <key> <port>
Connects to dec_server to decrypt text

    - The ciphertext argument is the name of a file containing the text to decrypt in the current directory
    - The key argument is the name of a file containing a key in the current directory
    - The port parameter is the port that dec_client should try to connect to dec_server on

    - Run validation and output appropriate error to stderr. Check:
        - There are no invalid characters in the ciphertext file
        - There are no invalid characters in the key file
        - The key file has at least as many characters as the ciphertext file
    - Connects to dec_server
        - If trying to connect to enc_server, reject and report rejection to stderr
        - If can't connect to dec_server, report to stderr with attempted port, set exit value to 2, and terminate
    - Passes key and ciphertext to dec_server
    - When dec_client receives plaintext from dec_server, outputs to stdout
    - Upon successfully running, sets exit value to 0 and exits
*/