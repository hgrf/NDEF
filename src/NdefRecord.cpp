#include "NdefRecord.h"

NdefRecord::NdefRecord()
{
    _tnf = NdefRecord::TNF_EMPTY;
    _typeLength = 0;
    _payloadLength = 0;
    _idLength = 0;
    _type = NULL;
    _payload = NULL;
    _id = NULL;
}

NdefRecord::NdefRecord(const NdefRecord& rhs)
{
    _tnf = rhs._tnf;
    _typeLength = rhs._typeLength;
    _payloadLength = rhs._payloadLength;
    _idLength = rhs._idLength;
    _type = NULL;
    _payload = NULL;
    _id = NULL;

    if (_typeLength)
    {
        _type = (byte*)malloc(_typeLength);
        memcpy(_type, rhs._type, _typeLength);
    }

    if (_payloadLength)
    {
        _payload = (byte*)malloc(_payloadLength);
        memcpy(_payload, rhs._payload, _payloadLength);
    }

    if (_idLength)
    {
        _id = (byte*)malloc(_idLength);
        memcpy(_id, rhs._id, _idLength);
    }

}

NdefRecord::~NdefRecord()
{
    free(_type);
    free(_payload);
    free(_id);
}

NdefRecord& NdefRecord::operator=(const NdefRecord& rhs)
{
    //Serial.println("NdefRecord ASSIGN");

    if (this != &rhs)
    {
        // free existing
        free(_type);
        free(_payload);
        free(_id);

        _tnf = rhs._tnf;
        _typeLength = rhs._typeLength;
        _payloadLength = rhs._payloadLength;
        _idLength = rhs._idLength;

        if (_typeLength)
        {
            _type = (byte*)malloc(_typeLength);
            if(_type)
                memcpy(_type, rhs._type, _typeLength);
            else
                Serial.println("No type malloc");
        }
        else
        {
            _type = NULL;
        }

        if (_payloadLength)
        {
            _payload = (byte*)malloc(_payloadLength);
            if(_payload)
                memcpy(_payload, rhs._payload, _payloadLength);
            else
                Serial.println("No type malloc");
        }
        else
        {
            _payload = NULL;
        }

        if (_idLength)
        {
            _id = (byte*)malloc(_idLength);
            if(_id)
                memcpy(_id, rhs._id, _idLength);
            else
                Serial.println("No type malloc");
        }
        else
        {
            _id = NULL;
        }
    }
    return *this;
}

// size of records in bytes
unsigned int NdefRecord::getEncodedSize()
{
    unsigned int size = 2; // tnf + typeLength
    if (_payloadLength > 0xFF)
    {
        size += 4;
    }
    else
    {
        size += 1;
    }

    if (_idLength)
    {
        size += 1;
    }

    size += (_typeLength + _payloadLength + _idLength);

    return size;
}

void NdefRecord::encode(byte *data, bool firstRecord, bool lastRecord)
{
    // assert data > getEncodedSize()

    uint8_t* data_ptr = &data[0];

    *data_ptr = _getTnfByte(firstRecord, lastRecord);
    data_ptr += 1;

    *data_ptr = _typeLength;
    data_ptr += 1;

    if (_payloadLength <= 0xFF) {  // short record
        *data_ptr = _payloadLength;
        data_ptr += 1;
    } else { // long format
        // 4 bytes but we store length as an int
        data_ptr[0] = 0x0; // (_payloadLength >> 24) & 0xFF;
        data_ptr[1] = 0x0; // (_payloadLength >> 16) & 0xFF;
        data_ptr[2] = (_payloadLength >> 8) & 0xFF;
        data_ptr[3] = _payloadLength & 0xFF;
        data_ptr += 4;
    }

    if (_idLength)
    {
        *data_ptr = _idLength;
        data_ptr += 1;
    }

    //Serial.println(2);
    memcpy(data_ptr, _type, _typeLength);
    data_ptr += _typeLength;

    if (_idLength)
    {
        memcpy(data_ptr, _id, _idLength);
        data_ptr += _idLength;
    }
    
    memcpy(data_ptr, _payload, _payloadLength);
    data_ptr += _payloadLength;
}

byte NdefRecord::_getTnfByte(bool firstRecord, bool lastRecord)
{
    int value = _tnf;

    if (firstRecord) { // mb
        value = value | 0x80;
    }

    if (lastRecord) { //
        value = value | 0x40;
    }

    // chunked flag is always false for now
    // if (cf) {
    //     value = value | 0x20;
    // }

    if (_payloadLength <= 0xFF) {
        value = value | 0x10;
    }

    if (_idLength) {
        value = value | 0x8;
    }

    return value;
}

NdefRecord::TNF NdefRecord::getTnf()
{
    return _tnf;
}

void NdefRecord::setTnf(NdefRecord::TNF tnf)
{
    _tnf = tnf;
}

unsigned int NdefRecord::getTypeLength()
{
    return _typeLength;
}

unsigned int NdefRecord::getPayloadLength()
{
    return _payloadLength;
}

unsigned int NdefRecord::getIdLength()
{
    return _idLength;
}

const byte* NdefRecord::getType()
{
    return _type;
}

void NdefRecord::setType(const byte *type, const unsigned int numBytes)
{
    free(_type);

    _type = (uint8_t*)malloc(numBytes);
    memcpy(_type, type, numBytes);
    _typeLength = numBytes;
}

const byte* NdefRecord::getPayload()
{
    return _payload;
}

void NdefRecord::setPayload(const byte *payload, const int numBytes)
{
    free(_payload);

    _payload = (byte*)malloc(numBytes);
    memcpy(_payload, payload, numBytes);
    _payloadLength = numBytes;
}

void NdefRecord::setPayload(const byte *header, const int headerLength, const byte *payload, const int payloadLength)
{
    free(_payload);

    _payload = (byte*)malloc(headerLength+payloadLength);
    memcpy(_payload, header, headerLength);
    memcpy(_payload+headerLength, payload, payloadLength);
    _payloadLength = headerLength+payloadLength;
}

const byte* NdefRecord::getId()
{
    return _id;
}

void NdefRecord::setId(const byte *id, const unsigned int numBytes)
{
    free(_id);

    _id = (byte*)malloc(numBytes);
    memcpy(_id, id, numBytes);
    _idLength = numBytes;
}

// c.f. https://www.oreilly.com/library/view/beginning-nfc/9781449324094/ch04.html
// c.f. https://www.oreilly.com/library/view/beginning-nfc/9781449324094/apa.html
String NdefRecord::asUri()
{
    if (_tnf != TNF_WELL_KNOWN)
    {
        Console.println(F("NdefRecord::asUri not a TNF_WELL_KNOWN"));
        return String();
    }

    if (_typeLength < 1)
    {
        Console.println(F("NdefRecord::asUri typeLength < 1"));
        return String();
    }

    if (_type[0] != RTD_URI && _type[0] != RTD_SMART_POSTER)
    {
        Console.print(F("NdefRecord::asUri not RTD_URI or RTD_SMART_POSTER: "));
        Console.println((int) _type[0]);
        return String();
    }

    if (_payloadLength < 1)
    {
        Console.println(F("NdefRecord::asUri payloadLength < 1"));
        return String();
    }

    String uri;
    if (_type[0] == RTD_URI) {
        String uriPrefixes[] = {
            "", "http://www.", "https://www.", "http://", "https://", "tel:", // ... to be completed
        };

        if (_payload[0] >= sizeof(uriPrefixes) / sizeof(uriPrefixes[0]))
        {
            Console.print(F("NdefRecord::asUri prefix out of range: "));
            Console.println((int) _payload[0]);
            return String();
        }

        uri += uriPrefixes[_payload[0]];
        for (unsigned int i = 1; i < _payloadLength; i++)
        {
            if (_payload[i] < 0x20 || _payload[i] > 0x7E)
            {
                return String();
            }
            uri += (char)_payload[i];
        }
    } else if (_type[0] == RTD_SMART_POSTER) {
        // c.f. https://berlin.ccc.de/~starbug/felica/NFCForum-SmartPoster_RTD_1.0.pdf
        // TODO: check if _type[1] == 'p'
        // TODO: check if SR or not SR
        // TODO: confirm record type
        // TODO: parse abbreviation
        int uriLength = _payload[2];
        Console.print(F("NdefRecord::asUri uriLength: "));
        Console.println(uriLength);
        for (unsigned int i = 5; i < 5 + uriLength - 1; i++)
        {
            if (_payload[i] < 0x20 || _payload[i] > 0x7E)
            {
                Console.print(F("NdefRecord::asUri invalid character: "));
                Console.println((int) _payload[i]);
                return String();
            }
            uri += (char)_payload[i];
        }
    }

    return uri;
}

#ifdef NDEF_USE_SERIAL

void NdefRecord::print()
{
    Serial.println(F("  NDEF Record"));
    Serial.print(F("    TNF 0x"));Serial.print(_tnf, HEX);Serial.print(" ");
    switch (_tnf) {
    case TNF_EMPTY:
        Serial.println(F("Empty"));
        break;
    case TNF_WELL_KNOWN:
        Serial.println(F("Well Known"));
        break;
    case TNF_MIME_MEDIA:
        Serial.println(F("Mime Media"));
        break;
    case TNF_ABSOLUTE_URI:
        Serial.println(F("Absolute URI"));
        break;
    case TNF_EXTERNAL_TYPE:
        Serial.println(F("External"));
        break;
    case TNF_UNKNOWN:
        Serial.println(F("Unknown"));
        break;
    case TNF_UNCHANGED:
        Serial.println(F("Unchanged"));
        break;
    case TNF_RESERVED:
        Serial.println(F("Reserved"));
        break;
    }
    Serial.print(F("    Type Length 0x"));Serial.print(_typeLength, HEX);Serial.print(" ");Serial.println(_typeLength);
    Serial.print(F("    Payload Length 0x"));Serial.print(_payloadLength, HEX);;Serial.print(" ");Serial.println(_payloadLength);
    if (_idLength)
    {
        Serial.print(F("    Id Length 0x"));Serial.println(_idLength, HEX);
    }
    Serial.print(F("    Type "));PrintHexChar(_type, _typeLength);
    // TODO chunk large payloads so this is readable
    Serial.print(F("    Payload "));PrintHexChar(_payload, _payloadLength);
    if (_idLength)
    {
        Serial.print(F("    Id "));PrintHexChar(_id, _idLength);
    }
    Serial.print(F("    Record is "));Serial.print(getEncodedSize());Serial.println(" bytes");

}
#endif
