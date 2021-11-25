/*
Usage: dec_server <port>
Performs decryption via one-time pad

    - Listens on specified port
    - Outputs error when executed if it can't run due to a network error like unavailable port
    - When connection is made:
        - Calls accept() to generate socket used for communication 
        - Uses a separate process the rest of the servicing for this connection on the new socket
        - Child process verifies it is communicating with dec_client
        - Child receives ciphertext and a key from dec_client
        - Child sends the plaintext back to dec_client
    - Must support up to 5 concurrent socket connections
    - Outputs errors to stderr and continues to run
*/
