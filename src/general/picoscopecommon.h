#ifndef PICOSCOPECOMMON_H
#define PICOSCOPECOMMON_H

enum class PSRange { R50mV,
                     R100mV,
                     R200mV,
                     R500mV,
                     R1V,
                     R2V,
                     R5V,
                     R10V,
                     R20V,
                     NONE
                   };

enum class PSCoupling { AC,
                        DC1M,
                        DC50R,
                        NONE
                      };

class PicoScopeCommon
{
public:

    static unsigned int PSRangeToInt(PSRange range)
    {
        switch(range)
        {
        case PSRange::R50mV:    return 1;   break;
        case PSRange::R100mV:   return 2;   break;
        case PSRange::R200mV:   return 3;   break;
        case PSRange::R500mV:   return 4;   break;
        case PSRange::R1V:      return 5;   break;
        case PSRange::R2V:      return 6;   break;
        case PSRange::R5V:      return 7;   break;
        case PSRange::R10V:     return 8;   break;
        case PSRange::R20V:     return 9;   break;

        case PSRange::NONE:     return 0;   break;
        default:                return 0;   break;
        }
    }

    static PSRange intToPSRange(unsigned int range)
    {
        switch(range)
        {
        case 1:  return PSRange::R50mV;  break;
        case 2:  return PSRange::R100mV; break;
        case 3:  return PSRange::R200mV; break;
        case 4:  return PSRange::R500mV; break;
        case 5:  return PSRange::R1V;    break;
        case 6:  return PSRange::R2V;    break;
        case 7:  return PSRange::R5V;    break;
        case 8:  return PSRange::R10V;   break;
        case 9:  return PSRange::R20V;   break;

        case 0:  return PSRange::NONE;   break;
        default: return PSRange::NONE;   break;
        }
    }

    static unsigned int PSCouplingToInt(PSCoupling coupling)
    {
        switch(coupling)
        {
        case PSCoupling::AC:    return 1;   break;
        case PSCoupling::DC1M:  return 2;   break;
        case PSCoupling::DC50R: return 3;   break;

        case PSCoupling::NONE:  return 0;   break;
        default:                return 0;   break;
        }
    }

    static PSCoupling intToPSCoupling(unsigned int coupling)
    {
        switch(coupling)
        {
        case 1:  return PSCoupling::AC;      break;
        case 2:  return PSCoupling::DC1M;    break;
        case 3:  return PSCoupling::DC50R;   break;

        case 0:  return PSCoupling::NONE;    break;
        default: return PSCoupling::NONE;    break;
        }
    }

    static double PSRangeToDouble(PSRange range)
    {
        switch(range)
        {
        case PSRange::R50mV:    return 0.05;    break;
        case PSRange::R100mV:   return 0.1;     break;
        case PSRange::R200mV:   return 0.2;     break;
        case PSRange::R500mV:   return 0.5;     break;
        case PSRange::R1V:      return 1.0;     break;
        case PSRange::R2V:      return 2.0;     break;
        case PSRange::R5V:      return 5.0;     break;
        case PSRange::R10V:     return 10.0;    break;
        case PSRange::R20V:     return 20.0;    break;

        case PSRange::NONE:     return 0.0;     break;
        default:                return 0.0;     break;
        }
    }
};

#endif // PICOSCOPECOMMON_H
