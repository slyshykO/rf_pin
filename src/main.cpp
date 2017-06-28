/*******************************************************************************
*  file    : main.cpp
*  created : 27.09.2016
*  author  : Slysyk Oleksiy (alexSlyshyk@gmail.com)
*******************************************************************************/

#include <cstdio>
#include <cstdint>
#include <cstdlib>
#include <cerrno>
#include <cstring>
#include <unistd.h>
#include <bcm2835/bcm2835.h>
#include <wiringPi.h>
#include <wiringPiSPI.h>

#include <vector>
#include <string>
#include <array>
#include <memory>

#define APP_MODE_RECEIVE    0
#define APP_MODE_TRANSMIT   1

#define SPI_CHAN            0

namespace {
int APP_MODE = APP_MODE_TRANSMIT;
int PIN_N    = static_cast<RPiGPIOPin>(10);
std::string transmit_str;

void app_sleep(uint32_t msec)
{
    usleep(1000* msec);
}

constexpr uint32_t spi_speed = 500000;

} //namespace

class FileDescriptorHolder
{
public:
    explicit FileDescriptorHolder(int fd) :fd_(fd) {}
    ~FileDescriptorHolder()
    {
        if(fd_ > 0)
            close(fd_);
    }
    int fd() {return fd_;}
    operator int() {return fd();}

private:
    int fd_ = -1;
};

int mode_receive();
int mode_transmit();
int set_by_pin(const std::string& tr, uint32_t n);

int main(int argc, char *argv[])
{
    int c;
    while((c = getopt(argc, argv, "rt:p:")) != -1)
        {
            switch (c)
                {
                case 'r':
                    APP_MODE = APP_MODE_RECEIVE;
                break;
                case 't':
                    APP_MODE = APP_MODE_TRANSMIT;
                    transmit_str = std::string(optarg);
                break;
                case 'p':
                    {
                        printf("%s\n", optarg );
                        char *end;
                        int p = std::strtol(optarg, &end, 0);
                        if(end != optarg)
                            PIN_N = static_cast<RPiGPIOPin>(p);
                    }
                break;
                case ':':
                    fprintf(stderr,"%s\n", "Missing option." );
                    exit(EXIT_FAILURE);
                break;
                }
        }
//    if (!bcm2835_init())
//        {
//            printf("Fail:Can't init pins");
//            return EXIT_FAILURE;
//        }
    int res = EXIT_FAILURE;
    //fprintf(stdout,"App mode = %i\n", APP_MODE);
    switch(APP_MODE)
        {
        case APP_MODE_RECEIVE:
            res = mode_receive();
        break;
        case APP_MODE_TRANSMIT:
            res = mode_transmit();
        break;
        }

    return res;
}

int mode_receive()
{
    fprintf(stdout,"RECEIVE\n");

    wiringPiSetup();

    std::unique_ptr<FileDescriptorHolder> fd(new FileDescriptorHolder(wiringPiSPISetup (SPI_CHAN, spi_speed)));
    if (*fd < 0)
        {
            fprintf (stderr, "Can't open the SPI bus: %s\n", strerror (errno)) ;
            exit (EXIT_FAILURE) ;
        }

    std::vector<uint8_t> pin_log;
    pin_log.resize(1024 * 4);

    uint32_t delay = 1000000 / spi_speed;

    fprintf(stdout,"listen...\n");
    if (wiringPiSPIDataRW (SPI_CHAN, pin_log.data(), pin_log.size()) == -1)
        {
            fprintf(stderr,"SPI failure: %s\n", strerror (errno)) ;
            exit (EXIT_FAILURE) ;
        }

    fprintf(stdout,"parsing...                          \n");
    /*for(uint32_t i = 0; i < pin_log.size(); ++i)
        {
            auto b = pin_log[i];
            for(int j = 0; j < 8; ++j)
                {
                    if(((i*8+j)%100 == 0) && (i*8+j))
                        fprintf(stdout, "\n");
                    if(((b & (1 << j)) != 0))
                        fprintf(stdout, "1");
                    else
                        fprintf(stdout, "0");
                }

        }
    fprintf(stdout, "\n\n\n");*/

    enum parse {
        P_SEARCH,
        P_PARSE
    };
    parse pst = P_SEARCH;
    uint32_t z_cnt = 0;
    uint32_t o_cnt = 0;
    std::vector<int> ppp;
    ppp.reserve(1000);
    auto bit_cnt = [](uint8_t b)
    {
        uint8_t res = 0;
        for(int i = 0; i < 8; ++i)
            if(b & (1 << i))
                res++;
        return res;
    };

    delay = delay * 8;
    std::string res;
    std::vector<std::string> res_vec;
    for(auto b : pin_log)
        {
            //for(int i = 0; i < 8; ++i)
                {
                    //bool v = ((b & (1 << i)) != 0);
                    bool v = bit_cnt(b) > 3;
                    switch(pst)
                        {
                        case P_SEARCH:
                            if(v){z_cnt = 0; o_cnt++;}
                            else
                                {
                                    /*if(o_cnt > (200/delay))
                                        {
                                            pst = P_PARSE;
                                            data_cnt = 0;
                                            fprintf(stdout,"Found  %u\n", o_cnt);
                                        }*/
                                    if(o_cnt)
                                        ppp.push_back(o_cnt*delay);
                                    o_cnt = 0; z_cnt++;
                                    if(z_cnt > (6000/delay))
                                        {
                                            pst = P_PARSE;
                                        }
                                }
                        break;
                        case P_PARSE:
                            if(v)
                                {
                                    z_cnt = 0; o_cnt++;
                                }
                            else
                                {
                                    if(o_cnt > (600/delay))
                                        res.push_back('1');
                                    else if(o_cnt > (200/delay))
                                        res.push_back('0');

                                    if(o_cnt)
                                        ppp.push_back(o_cnt*delay);
                                    o_cnt = 0; z_cnt++;

                                    if(z_cnt > (1010/delay))
                                        {
                                            pst = P_SEARCH;
                                            if(res.size())
                                                {
                                                    res_vec.push_back(res);
                                                    res = "";
                                                }
                                        }
                                }
                        break;
                        default:
                            pst = P_SEARCH;
                        break;
                        }
                }
        }
    //fprintf(stdout, "\n");
    /*for(uint32_t i = 0; i < ppp.size(); ++i)
        {
            if(((i % 10) == 0) && (i > 0))
                fprintf(stdout, "\n");
            fprintf(stdout, "%04i ", ppp.at(i));
        }
    fprintf(stdout, "\n");*/

    fprintf(stdout, "--------------------------------\n");
    for(const auto& s : res_vec)
        if(s.size() > 5)
            fprintf(stdout, "%s\n", s.c_str());

    return EXIT_SUCCESS;
}

int mode_transmit()
{
    fprintf(stdout, "TRANSMIT:%s\n", transmit_str.c_str());

    wiringPiSetup();

    std::unique_ptr<FileDescriptorHolder> fd(new FileDescriptorHolder(wiringPiSPISetup (SPI_CHAN, spi_speed)));
    if (*fd < 0)
        {
            fprintf (stderr, "Can't open the SPI bus: %s\n", strerror (errno)) ;
            exit (EXIT_FAILURE) ;
        }

    std::vector<uint8_t> pin_log;
    pin_log.resize(1024 * 4);
    uint32_t delay = 1000000 / spi_speed;
    uint32_t bpms = 1024 / delay / 8; //количество байтов на микросекунду
    uint32_t zero_b = 20;//(double)bpms / 10. * 3.;
    uint32_t one_b = bpms - zero_b;
    uint32_t silence_len = bpms * 8;
    //преамбула silence
    uint8_t zz = 0xff;
    uint8_t oo = 0xff;
    for(uint32_t i = 0; i < silence_len; ++i)
        pin_log[i] = zz;
    for(uint32_t i = 0, cnt = silence_len; i < transmit_str.size(); ++i)
        {
            if(transmit_str[i] == '0')
                {
                    for(uint32_t k = 0; k < zero_b; ++k)
                        pin_log[cnt++] = oo;
                    for(uint32_t k = 0; k < one_b; ++k)
                        pin_log[cnt++] = zz;
                }
            else
                {
                    for(uint32_t k = 0; k < one_b; ++k)
                        pin_log[cnt++] = oo;
                    for(uint32_t k = 0; k < zero_b; ++k)
                        pin_log[cnt++] = zz;
                }
        }

    /*fprintf(stderr,"Send %u bytes\n", silence_len + transmit_str.size() * bpms) ;
    for(int i = 0; i < 100; ++i)
        if (wiringPiSPIDataRW (SPI_CHAN, pin_log.data(), silence_len + transmit_str.size() * bpms) == -1)
            {
                fprintf(stderr,"SPI failure: %s\n", strerror (errno)) ;
                exit (EXIT_FAILURE) ;
            }*/
    return set_by_pin(transmit_str, 100);

    return EXIT_SUCCESS;
}

int set_by_pin(const std::string& tr, uint32_t n)
{
    if (!bcm2835_init())
        {
            fprintf (stderr,"Fail:Can't init pins");
            return EXIT_FAILURE;
        }
    bcm2835_gpio_fsel   (PIN_N , BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_set_pud(PIN_N , BCM2835_GPIO_PUD_OFF);
    bcm2835_gpio_clr(PIN_N);
    usleep(1000);
    for(uint32_t i = 0; i < n; ++i)
        {
            bcm2835_gpio_clr(PIN_N);
            usleep(7000);
            //fprintf (stdout, "K:%s\n", tr.c_str());
            //fprintf (stdout, "S:");
            for(uint32_t s = 0; s < tr.size(); ++s)
                {
                    if(transmit_str.at(s) == '0')
                        {
                            bcm2835_gpio_set(PIN_N);
                            usleep(190);
                            bcm2835_gpio_clr(PIN_N);
                            usleep(680);
                            //fprintf (stdout, "0");
                        }
                    else
                        {
                            bcm2835_gpio_set(PIN_N);
                            usleep(680);
                            bcm2835_gpio_clr(PIN_N);
                            usleep(190);
                            //fprintf (stdout, "1");
                        }
                }
            //fprintf (stdout, "\n");
        }
    return EXIT_SUCCESS;
}

