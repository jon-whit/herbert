#pragma once

#include <Exceptions.h>


class InstrumentTypes
{
public:
    enum ExcitationColors
    {
        EXCOLOR_NONE,
        EXCOLOR_470,
        EXCOLOR_530,
        EXCOLOR_586,
        EXCOLOR_630,
    };
    enum EmissionColors
    {
        EMCOLOR_510,
        EMCOLOR_555,
        EMCOLOR_620,
        EMCOLOR_640,
        EMCOLOR_665,
        EMCOLOR_710,
    };

    static std::string getExcitationColorName(ExcitationColors color)
    {
        switch(color)
        {
        case EXCOLOR_NONE:  return "None";
        case EXCOLOR_470:   return "470";
        case EXCOLOR_530:   return "530";
        case EXCOLOR_586:   return "586";
        case EXCOLOR_630:   return "630";
        default: ITI_ERROR("Unknown Excitation Color");
        }
        return "";
    }

    static ExcitationColors getExcitationColorFromString(const std::string& color)
    {
        if(color == "None") return EXCOLOR_NONE;
        if(color == "470")  return EXCOLOR_470;
        if(color == "530")  return EXCOLOR_530;
        if(color == "586")  return EXCOLOR_586;
        if(color == "630")  return EXCOLOR_630;
        ITI_ERROR(("Unknown Excitation Color: '" + color + "'").c_str());
        return EXCOLOR_NONE;
    }

    static std::string getEmissionColorName(EmissionColors color)
    {
        switch(color)
        {
        case EMCOLOR_510:  return "510 (FAM)";
        case EMCOLOR_555:  return "555 (HEX)";
        case EMCOLOR_620:  return "620 (ROX)";
        case EMCOLOR_640:  return "640 (LC-640)";
        case EMCOLOR_665:  return "665 (Cy5)";
        case EMCOLOR_710:  return "710 (LC-705)";
        default: ITI_ERROR("Unknown Emission Color");
        }
        return "";
    }
};

