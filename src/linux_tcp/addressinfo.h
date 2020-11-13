/**
 *  AddressInfo.h
 *
 *  Utility wrapper around "getAddressInfo()"
 *
 *  @author Emiel Bruijntjes <emiel.bruijntjes@copernica.com>
 *  @copyright 2015 Copernica BV
 */

/**
 *  Dependencies
 */
#include <sys/time.h>

/**
 *  Include guard
 */
namespace AMQP {

/**
 *  Class definition
 */
class AddressInfo
{
private:
    /**
     *  The addresses
     *  @var struct AddressInfo
     */
    struct addrinfo *_info = nullptr;
    
    /**
     *  Vector of addrinfo pointers
     *  @var std::vector<struct addrinfo *>
     */
    std::vector<struct addrinfo *> _v;

public:
    /**
     *  Constructor
     *  @param  hostname
     *  @param  port
     *  @param  randomOrder
     */
    AddressInfo(const char *hostname, uint16_t port = 5672, bool randomOrder = false)
    {
        // store portnumber in buffer
        auto portnumber = std::to_string(port);
        
        // info about the lookup
        struct addrinfo hints;
        
        // set everything to zero
        memset(&hints, 0, sizeof(struct addrinfo));
        
        // set hints
        hints.ai_family = AF_UNSPEC;        // allow IPv4 or IPv6
        hints.ai_socktype = SOCK_STREAM;    // datagram socket/
        
        // get address of the server
        auto code = getaddrinfo(hostname, portnumber.data(), &hints, &_info);
        
        // was there an error
        if (code != 0) throw std::runtime_error(gai_strerror(code));
        
        // keep looping
        for (auto *current = _info; current; current = current->ai_next)
        {
            // store in vector
            _v.push_back(current);
        }

        // Do we want to have a random order of the addresses?
        // This may be useful since getaddrinfo is sorting the addresses on proximity
        // (e.g. https://lists.debian.org/debian-glibc/2007/09/msg00347.html),
        // which may break loadbalancing..
        if (randomOrder)
        {
            // We need to seed the random number generator. Normally time is taken
            // for this. Since we want to have randomness within a second we use
            // more precision via timeval
            struct timeval time; 
            gettimeofday(&time, nullptr);

            // We seed with the precision of miliseconds. 
            srand((time.tv_sec * 1000) + (time.tv_usec / 1000));  

            // shuffle the vector.    
            std::random_shuffle(_v.begin(), _v.end());
        }
    }

    /**
     *  Destructor
     */
    virtual ~AddressInfo()
    {
        // free address info
        freeaddrinfo(_info);
    }
    
    /**
     *  Size of the array
     *  @return size_t
     */
    size_t size() const
    {
        return _v.size();
    }
    
    /**
     *  Get reference to struct
     *  @param  index
     *  @return struct addrinfo*
     */
    const struct addrinfo *operator[](int index) const
    {
        // expose vector
        return _v[index];
    }
};

/**
 *  End of namespace
 */
}
