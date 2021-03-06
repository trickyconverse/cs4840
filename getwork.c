#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include <jansson.h>
#include <curl/curl.h>

#define BUFFER_SIZE  (256 * 1024)  /* 256 KB */
 // curl --user halffast.worker1:WyhZfpFS --data-binary '{ "id":"curltest", "method":"getwork", "params":[] }' -H 'content-type: text/plain;' http://localhost:8332 -i

//{"error": null, 
//  "id": "curltest",
//  "result":
//  {"hash1": "00000000000000000000000000000000000000000000000000000000000000000000008000000000000000000000000000000000000000000000000000010000",
//    "data": "000000029b39574cb8a5c25e94cdd844f7d3b942384af65fb264d5a000000000000000008adb353c754fe07c40341728c5efe6e3782d994e0e3ebcb4879106de94dd1db0533b2c461900db9900000000000000800000000000000000000000000000000000000000000000000000000000000000000000000000000080020000",
//    "target": "0000000000000000000000000000000000000000000000000000ffff00000000", 
//    "midstate": "49bc9ada38ba6b43814a3275edfa09b796be70080d5c641d32b80776aef13f78"}
//}


const char *req = "{ \"id\":\"1\", \"method\":\"getwork\", \"params\":[] }";

//defaults
const char *usrpwd = "halffast.worker1:WyhZfpFS";
const char *header = "content-type: text/plain;";
const char *pool_url = "http://localhost:8332/";
const char *lp_pool_url = "http://localhost:8332/lp";

static void die(const char *message)
{
    perror(message);
    exit(1);
}

static void json_die(const int error_line, const char *error_text)
{
    fprintf(stderr, "error: on line %d: %s\n", error_line, error_text);
    exit(1);
}

// write_result, write_response and request came from the jansson tutorial
// http://jansson.readthedocs.org/en/2.3/tutorial.html
struct write_result
{
    char *data;
    int pos;
};

static size_t write_response(void *ptr, size_t size, size_t nmemb, void *stream)
{
    struct write_result *result = (struct write_result *)stream;

    if(result->pos + size * nmemb >= BUFFER_SIZE - 1)
    {
        fprintf(stderr, "error: too small buffer\n");
        return 0;
    }

    memcpy(result->data + result->pos, ptr, size * nmemb);
    result->pos += size * nmemb;

    return size * nmemb;
}


//-i, --include       Include protocol headers in the output (H/F)
static char *request(const char *url, const char *bin_data)
{
    CURL *curl = NULL;
    CURLcode status;
    struct curl_slist *headers = NULL;
    char *data = NULL;
    long code;

    curl_global_init(CURL_GLOBAL_ALL);
    curl = curl_easy_init();
    if(!curl)
        goto error;

    data = malloc(BUFFER_SIZE);
    if(!data)
        goto error;

    struct write_result write_result = {
        .data = data,
        .pos = 0
    };

    
    //set up url
    curl_easy_setopt(curl, CURLOPT_URL, url);
    
    //set up bin_data the crux of the getwork request
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, ((void *) bin_data));
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, ((long)-1) );

    //set up usrpwd
    curl_easy_setopt(curl, CURLOPT_USERPWD, usrpwd);

    //set up headers from init response
    headers = curl_slist_append(headers, header);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    //set up writing repsonse
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_response);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &write_result);

    //preform the curl
    status = curl_easy_perform(curl);

    //error handle
    if(status != 0)
    {
        fprintf(stderr, "error: unable to request data from %s:\n", url);
        fprintf(stderr, "%s\n", curl_easy_strerror(status));
        goto error;
    }

    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &code);
    if(code != 200)
    {
        fprintf(stderr, "error: server responded with code %ld\n", code);
        goto error;
    }

    curl_easy_cleanup(curl);
    curl_slist_free_all(headers);
    curl_global_cleanup();

    /* zero-terminate the result */
    data[write_result.pos] = '\0';

    return data;

error:
    if(data)
        free(data);
    if(curl)
        curl_easy_cleanup(curl);
    if(headers)
        curl_slist_free_all(headers);
    curl_global_cleanup();
    return NULL;
}

// static void swap(char *a, char *b){ 
//   char temp;
//   temp = *a;
//   *a = *b;
//   *b = temp;
// }

//TESTS
// char test_thing[17] = "0123456701234567\0";
// printf("%s\n", endian_flip_32_bit_chunks(test_thing));
// static char *endian_flip_32_bit_chunks(char *input)
// {
//     //32 bits = 4*4 bytes = 4*4 chars
//     printf("%lu\n", strlen(input));
//     for (int i = 0; i < strlen(input); i += 8){   
//         swap(&input[i], &input[i+6]);
//         swap(&input[i+1], &input[i+7]);
//         swap(&input[i+2], &input[i+4]);
//         swap(&input[i+3], &input[i+5]);
//     }
//     return input;        
// }


int
main(int argc, char**argv){

    //getwork from network
    char *init_resp;
    json_t *json_resp;
    json_error_t json_error;

    json_int_t json_data_error_num;
    json_t *data_error = NULL, *result = NULL, *data = NULL, *target = NULL, *midstate = NULL;

    init_resp = request(pool_url, req);
    if(!init_resp){
        die("initial repsonse failed check args");
    }

    json_resp = json_loads(init_resp, 0, &json_error);

    if(!json_resp){
        json_die(json_error.line, json_error.text);
    }

    data_error = json_object_get(json_resp, "error");

    //these are json-rpc errors
    json_data_error_num = json_integer_value(data_error);
    if(json_data_error_num != 0){
        switch(json_data_error_num){
            case 20 : printf("Other/Unknown\n");            
            case 21 : printf("Job not found (=stale)\n");            
            case 22 : printf("Duplicate share\n");            
            case 23 : printf("Low difficulty share\n");            
            case 24 : printf("Unauthorized worker\n");            
            case 25 : printf("Not subscribed\n");
            default : printf("strange error google resuslts\n");            
        }
        fprintf(stderr, "%s\n", init_resp);
        die("error in result from initial response");
    }

    //{"error": null, 
    //  "id": "curltest",
    //  "result":
    //  {"hash1": "00000000000000000000000000000000000000000000000000000000000000000000008000000000000000000000000000000000000000000000000000010000",
    //    "data": "
    // 000000029b39574cb8a5c25e94cdd844f7d3b942384af65fb264d5a000000000
    // 000000008adb353c754fe07c40341728c5efe6e3782d994e0e3ebcb4879106de
    // 94dd1db0533b2c461900db990000000000000080000000000000000000000000
    // 0000000000000000000000000000000000000000000000000000000080020000
    //    "target": "0000000000000000000000000000000000000000000000000000ffff00000000", 
    //    "midstate": "49bc9ada38ba6b43814a3275edfa09b796be70080d5c641d32b80776aef13f78"}
    //}

    result = json_object_get(json_resp, "result");
    data = json_object_get(result, "data");
    target = json_object_get(result, "target");
    midstate = json_object_get(result, "midstate");

    // printf("%s\n", json_string_value(midstate));

    if(!result || !data || !target || !midstate){
        fprintf(stderr, "%s\n", init_resp);
        die("initial repsonse format error");
    }


    // !! prep work
    // prefered that data is sent as big endien

    // data is in little endien hex format
    printf("%s\n", json_string_value(data));
    printf("%s\n", endian_flip_32_bit_chunks((char *)json_string_value(data)));
    data must be back through the net work as little endien

    // !! pass data down to miners

    //used for error printing
    printf("%s\n", init_resp); //for testing

    json_decref(json_resp);
    free(init_resp);

    // !! create thread for listening for success from miners
    // upon success send a proof of work to the network with params set to solution
    // eg. {"method":"getwork","params":["0000000141a0e898cf6554fd344a37b2917a6c7a6561c20733b09c8000009eef00000000d559e21 882efc6f76bbfad4cd13639f4067cd904fe4ecc3351dc9cc5358f1cd54db84e7a1b00b5acba97b6 0400000080000000000000000000000000000000000000000000000000000000000000000000000 0000000000080020000"],"id":1}
    // data must be back through the net work as little endien

    //loop for long polling
    while(1){
        
        init_resp = request(lp_pool_url, req); //this will hang with long pool until fresh data is ready for work :)
        
        if(!init_resp){
            die("initial repsonse failed check args");
        }

        json_resp = json_loads(init_resp, 0, &json_error);

        if(!json_resp){
            json_die(json_error.line, json_error.text);
        }

        data_error = json_object_get(json_resp, "error");

        //these are json-rpc errors
        json_data_error_num = json_integer_value(data_error);
        if(json_data_error_num != 0){
            switch(json_data_error_num){
                case 20 : printf("Other/Unknown\n");            
                case 21 : printf("Job not found (=stale)\n");            
                case 22 : printf("Duplicate share\n");            
                case 23 : printf("Low difficulty share\n");            
                case 24 : printf("Unauthorized worker\n");            
                case 25 : printf("Not subscribed\n");
                default : printf("strange error google resuslts\n");            
            }
            fprintf(stderr, "%s\n", init_resp);
            die("error in result from initial response");
        }

        result = json_object_get(json_resp, "result");
        data = json_object_get(result, "data");
        target = json_object_get(result, "target");
        midstate = json_object_get(result, "midstate");

        if(!result || !data || !target || !midstate){
            fprintf(stderr, "%s\n", init_resp);
            die("initial repsonse format error");
        }

        // !! prep work
        //prefered that data is sent as big endien
        // data is in little endien hex format
        // printf("%s\n", json_string_value(data));
        // printf("%s\n", endian_flip_32_bit_chunks((char *)json_string_value(data)));

        // !! pass data down to miners

        //send down the second part with the 16 bits padded
        //send the midstate down is it little endian probably
        
        printf("%s\n", init_resp); //for testing

        json_decref(json_resp);
        free(init_resp);
    }

    return 0;
}