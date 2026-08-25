#include <stdio.h> // printf
#include <wchar.h> // wprintf

#include <unistd.h>

#include <hidapi/hidapi.h>

#define MAX_STR 255

int buf_size = 64;

int main(int argc, char* argv[])
{
    buf_size++;
    int res;
    unsigned char buf[65];
    wchar_t wstr[MAX_STR];
    hid_device* handle;
    int i;

    // Initialize the hidapi library
    res = hid_init();

    // Open the device using the VID, PID,
    // and optionally the Serial number.
    handle = hid_open(0x4d8, 0x102, NULL);

    int num = 321;
    char snum[5];

    // convert 123 to string [buf]
    sprintf(snum, "%d", num);

    // print our string
    printf("test%s\n", snum);

    if (!handle)
    {
        printf("Could not find attached rev burner\n");
        res = hid_exit();
        return 0;
    }

    // Read the Manufacturer String
    res = hid_get_manufacturer_string(handle, wstr, MAX_STR);
    wprintf(L"Manufacturer String: %s\n", wstr);

    // Read the Product String
    res = hid_get_product_string(handle, wstr, MAX_STR);
    wprintf(L"Product String: %s\n", wstr);

    // Read the Serial Number String
    res = hid_get_serial_number_string(handle, wstr, MAX_STR);
    wprintf(L"Serial Number String: (%d) %s\n", wstr[0], wstr);


    int rpm_array[21] = {41574, 48580, 54308, 58612, 60160, 61113, 61916, 62347, 62777, 63089, 63243, 63505, 63673, 63778, 63908, 64002, 64084, 64140, 64175, 64224, 64313};

    buf[0] = 0x00;
    buf[1] = 0x00;
    buf[2] = 0x00;
    buf[3] = 0x00;
    res = hid_write(handle, buf, buf_size);

    for (i = 0; i<21; i++)
    {
        int rpms = i * 500;

        fprintf(stderr, "Setting RPM to %i hex as decimal %i\n", rpms, rpm_array[i]);

        unsigned char bytes[buf_size];
        unsigned long nnnnn = rpm_array[i];
        unsigned long xlong = 0;
        for (int x = 0; x < buf_size; x++)
        {
            bytes[x] = 0x00;
        }

        bytes[3] = (nnnnn >> 8) & 0xFF;
        bytes[2] = nnnnn & 0xFF;

        FILE* write_ptr;
        if ( i == 0 )
        {
            write_ptr = fopen("test0.bin","wb");
        }

        if ( i == 1 )
        {
            write_ptr = fopen("test1.bin","wb");
        }

        if ( i == 2 )
        {
            write_ptr = fopen("test2.bin","wb");
        }

        if ( i == 3 )
        {
            write_ptr = fopen("test3.bin","wb");
        }

        if ( i == 4 )
        {
            write_ptr = fopen("test4.bin","wb");
        }

        if ( i == 5 )
        {
            write_ptr = fopen("test5.bin","wb");
        }

        if ( i == 6 )
        {
            write_ptr = fopen("test6.bin","wb");
        }

        if ( i == 7 )
        {
            write_ptr = fopen("test7.bin","wb");
        }

        if ( i == 8 )
        {
            write_ptr = fopen("test8.bin","wb");
        }

        if ( i == 9 )
        {
            write_ptr = fopen("test9.bin","wb");
        }

        if ( i == 10 )
        {
            write_ptr = fopen("test10.bin","wb");
        }

        if ( i == 11 )
        {
            write_ptr = fopen("test11.bin","wb");
        }

        if ( i == 12 )
        {
            write_ptr = fopen("test12.bin","wb");
        }

        if ( i == 13 )
        {
            write_ptr = fopen("test13.bin","wb");
        }

        if ( i == 14 )
        {
            write_ptr = fopen("test14.bin","wb");
        }

        if ( i == 15 )
        {
            write_ptr = fopen("test15.bin","wb");
        }

        if ( i == 16 )
        {
            write_ptr = fopen("test16.bin","wb");
        }

        if ( i == 17 )
        {
            write_ptr = fopen("test17.bin","wb");
        }

        if ( i == 18 )
        {
            write_ptr = fopen("test18.bin","wb");
        }

        if ( i == 19 )
        {
            write_ptr = fopen("test19.bin","wb");
        }

        if ( i == 20 )
        {
            write_ptr = fopen("test20.bin","wb");
        }

        if ( i == 21 )
        {
            write_ptr = fopen("test21.bin","wb");
        }

        if ( i == 22 )
        {
            write_ptr = fopen("test22.bin","wb");
        }

        if ( i == 23 )
        {
            write_ptr = fopen("test23.bin","wb");
        }

        if ( i == 24 )
        {
            write_ptr = fopen("test24.bin","wb");
        }

        fwrite(bytes,sizeof(bytes),1,write_ptr);
        fclose(write_ptr);

        if (handle)
        {
            res = hid_write(handle, bytes, buf_size);
        }

        sleep(4);
    }

    buf[0] = 0x00;
    buf[1] = 0x00;
    buf[2] = 0x00;
    buf[3] = 0x00;
    res = hid_write(handle, buf, buf_size);

    sleep(2);
    // Close the device
    hid_close(handle);

    // Finalize the hidapi library
    res = hid_exit();

    return 0;
}
