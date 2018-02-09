#include <rsys/logging.h>
#include <iomanip>

using namespace Executor;

int logging::nestingLevel = 0;
static bool loggingEnabled = false;
std::unordered_map<void*, const char*> logging::namedThings;

void logging::resetNestingLevel()
{
    nestingLevel  = 0;
}
void logging::indent()
{
    for(int i = 0; i < nestingLevel; i++)
        std::cout << "  ";
}

bool logging::enabled()
{
    return loggingEnabled;
}

void logging::setEnabled(bool e)
{
    loggingEnabled = e;
}

bool logging::loggingActive()
{
    return nestingLevel == 0;
}

void logging::logEscapedChar(unsigned char c)
{
    if(c == '\'' || c == '\"' || c == '\\')
        std::cout << '\\' << c;
    else if(std::isprint(c))
        std::cout << c;
    else
        std::cout << "\\0" << std::oct << (unsigned)c << std::dec;
}

bool logging::canConvertBack(const void* p)
{
    if(!p || p == (const void*) -1)
        return true;
#if SIZEOF_VOID_P == 4
    bool valid = true;
#else
    bool valid = false;
    for(int i = 0; i < 4; i++)
    {
        if((uintptr_t)p >= ROMlib_offsets[i] &&
            (uintptr_t)p < ROMlib_offsets[i] + ROMlib_sizes[i])
            valid = true;
    }
#endif
    return valid;
}

bool logging::validAddress(const void* p)
{
    if(!p)
        return false;
    if( (uintptr_t)p & 1 )
        return false;
#if SIZEOF_VOID_P == 4
    bool valid = true;
#else
    bool valid = false;
    for(int i = 0; i < 4; i++)
    {
        if((uintptr_t)p >= ROMlib_offsets[i] &&
            (uintptr_t)p < ROMlib_offsets[i] + ROMlib_sizes[i])
            valid = true;
    }
#endif
    if(!valid)
        return false;

    return true;
}

bool logging::validAddress(syn68k_addr_t p)
{
    if(p == 0 || (p & 1))
        return false;
    return validAddress(SYN68K_TO_US(p));
}


void logging::logValue(char x)
{
    std::cout << (int)x;
    std::cout << " = '";
    logEscapedChar(x);
    std::cout << '\'';
}
void logging::logValue(unsigned char x)
{
    std::cout << (int)x;
    if(std::isprint(x))
        std::cout << " = '" << x << '\'';
}
void logging::logValue(signed char x)
{
    std::cout << (int)x;
    if(std::isprint(x))
        std::cout << " = '" << x << '\'';
}
void logging::logValue(int16_t x) { std::cout << x; }
void logging::logValue(uint16_t x) { std::cout << x; }
void logging::logValue(int32_t x)
{
    std::cout << x << " = '";
    logEscapedChar((x >> 24) & 0xFF);
    logEscapedChar((x >> 16) & 0xFF);
    logEscapedChar((x >> 8) & 0xFF);
    logEscapedChar(x & 0xFF);
    std::cout << "'";
}
void logging::logValue(uint32_t x)
{
    std::cout << x << " = '";
    logEscapedChar((x >> 24) & 0xFF);
    logEscapedChar((x >> 16) & 0xFF);
    logEscapedChar((x >> 8) & 0xFF);
    logEscapedChar(x & 0xFF);
    std::cout << "'";
}
void logging::logValue(unsigned char* p)
{
    std::cout << "0x" << std::hex << US_TO_SYN68K_CHECK0_CHECKNEG1(p) << std::dec;
    if(validAddress(p) && validAddress(p+256))
    {
        std::cout << " = \"\\p";
        for(int i = 1; i <= p[0]; i++)
            logEscapedChar(p[i]);
        std::cout << '"';
    }
}
void logging::logValue(const void* p)
{
    if(canConvertBack(p))
        std::cout << "0x" << std::hex << US_TO_SYN68K_CHECK0_CHECKNEG1(p) << std::dec;
    else
        std::cout << "?";
}
void logging::logValue(void* p)
{
    if(canConvertBack(p))
        std::cout << "0x" << std::hex << US_TO_SYN68K_CHECK0_CHECKNEG1(p) << std::dec;
    else
        std::cout << "?";
}
void logging::logValue(ProcPtr p)
{
    if(canConvertBack(p))
        std::cout << "0x" << std::hex << US_TO_SYN68K_CHECK0_CHECKNEG1(p) << std::dec;
    else
        std::cout << "?";
}


void logging::dumpRegsAndStack()
{
    std::cout << std::hex << /*std::showbase <<*/ std::setfill('0');
    std::cout << "D0=" << std::setw(8) << EM_D0 << " ";
    std::cout << "D1=" << std::setw(8) << EM_D1 << " ";
    std::cout << "A0=" << std::setw(8) << EM_A0 << " ";
    std::cout << "A1=" << std::setw(8) << EM_A1 << " ";
    //std::cout << std::noshowbase;
    std::cout << "Stack: ";
    uint8_t *p = (uint8_t*)SYN68K_TO_US(EM_A7);
    for(int i = 0; i < 12; i++)
        std::cout << std::setfill('0') << std::setw(2) << (unsigned)p[i] << " ";
    std::cout << std::dec;
}


syn68k_addr_t logging::untypedLoggedFunction(syn68k_addr_t (*fptr)(syn68k_addr_t, void *), syn68k_addr_t addr, void * param)
{
    const char *fname = namedThings.at((void*)fptr);
    if(loggingActive())
    {
        std::cout.clear();
        indent();
        std::cout << fname << " ";
        dumpRegsAndStack();
        std::cout << std::endl;
    }
    nestingLevel++;
    syn68k_addr_t retaddr = (*fptr)(addr, param);
    nestingLevel--;
    if(loggingActive())
    {
        indent();
        std::cout << "returning: " << fname << " ";
        dumpRegsAndStack();
        std::cout << std::endl << std::flush;
    }
    return retaddr;
}
