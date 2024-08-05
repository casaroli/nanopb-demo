#include <stdio.h>
#include <stdlib.h>

#include <pb_encode.h>
#include <pb_decode.h>

#include "demo.pb.h"

#define BUF_LEN (256)

bool write_value(pb_ostream_t *stream, const pb_field_iter_t *field, void * const *arg)
{
    printf("encode callback called!!!\n");
    char *str = "my string value";
    if (!pb_encode_tag_for_field(stream, field))
        return false;

    return pb_encode_string(stream, (uint8_t*)str, strlen(str));
}

bool read_value(pb_istream_t *stream, const pb_field_iter_t *field, void **arg)
{
    printf("decode callback called, bytes_left=%jd!!!\n", stream->bytes_left);

    unsigned char *buf = calloc(1, stream->bytes_left+1);

    if (!buf) {
        printf("ERROR\n");
        return false;
    }

    if (!pb_read(stream, buf, stream->bytes_left)) {
        printf("ERR pb read\n");
        free(buf);
        return false;
    }

    *(unsigned char **)*arg = buf;

    return true;
}

void hexdump(uint8_t *buf, size_t len) {
    size_t i;
    for (i = 0; i < len; i++) {
        if (!(i%16)) {
            printf("%04jx: ", i);
        }
        printf("%02x ", buf[i]);
        if (i%16 == 15 || i == len) {
            printf("\n");
        }
    }
}

int main(void) {
    uint8_t *buffer;
    size_t message_length;
    bool status;
    int ret = 0;

    /* Allocate space on the stack to store the message data. */

    buffer = calloc(1, BUF_LEN);

    if (!buffer)
    {
        printf("malloc failed\n");
        return 1;
    }
    
    /* Encode message */
    {
        /* Nanopb generates simple struct definitions for all the messages.
         * - check out the contents of simple.pb.h!
         * It is a good idea to always initialize your structures
         * so that you do not have garbage data from RAM in there.
         */
        MessageRequest message = MessageRequest_init_zero;
        
        /* Create a stream that will write to our buffer. */
        pb_ostream_t stream = pb_ostream_from_buffer(buffer, BUF_LEN);
        
        /* Fill in the lucky number */
        strcpy(message.key, "zirigdum1");

        message.value.funcs.encode = write_value;
        
        /* Now we are ready to encode the message! */
        status = pb_encode(&stream, MessageRequest_fields, &message);
        message_length = stream.bytes_written;
        
        /* Then just check for any errors. */
        if (!status)
        {
            printf("Encoding failed: %s\n", PB_GET_ERROR(&stream));
            ret = 1;
            goto end;
        }
    }

    hexdump(buffer, BUF_LEN);

    /*  decode it immediately */
    
    {
        /* Allocate space for the decoded message. */
        MessageRequest message = MessageRequest_init_zero;
        
        /* Create a stream that reads from the buffer. */
        pb_istream_t stream = pb_istream_from_buffer(buffer, message_length);

        char *msgvalue;
        message.value.funcs.decode = read_value;
        message.value.arg = &msgvalue;
        
        /* Now we are ready to decode the message. */
        status = pb_decode(&stream, MessageRequest_fields, &message);
        
        /* Check for errors... */
        if (!status)
        {
            printf("Decoding failed: %s\n", PB_GET_ERROR(&stream));
            ret = 1;
            goto end;
        }
        
        /* Print the data contained in the message. */
        printf("The key is '%s' and the value is '%s'!\n", message.key, msgvalue);

        free(msgvalue);
    }

end:
    free(buffer);
    return ret;
}