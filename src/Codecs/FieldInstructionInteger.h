// Copyright (c) 2009, Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#ifdef _MSC_VER
# pragma once
#endif
#ifndef FIELDINSTRUCTIONINTEGER_H
#define FIELDINSTRUCTIONINTEGER_H
#include <Codecs/FieldInstruction.h>
#include <Codecs/Decoder.h>
#include <Codecs/Encoder.h>
#include <Codecs/DataSource.h>
#include <Codecs/DataDestination.h>
#include <Codecs/Dictionary.h>
#include <Messages/Message.h>
#include <Messages/Field.h>

#include <Common/Profiler.h>

using namespace ::QuickFAST;
using namespace ::QuickFAST::Codecs;

namespace QuickFAST{
  namespace Codecs{

    /// @brief A basic implementation for all integral types.
    ///
    /// Used for &lt;int32> &lt;uint32> &lt;int64> &lt;uint64> fields.
    template<typename INTEGER_TYPE, typename FIELD_CLASS, bool SIGNED>
    class FieldInstructionInteger : public FieldInstruction
    {
    public:
      /// @brief construct with a name and a namespace
      /// @param name is the local name
      /// @param fieldNamespace is the namespace to qualify this name
      FieldInstructionInteger(
        const std::string & name,
        const std::string & fieldNamespace);

      /// @brief construct anonomous field instruction
      FieldInstructionInteger();

      /// @brief a typical virtual destructor.
      virtual ~FieldInstructionInteger();

      virtual void interpretValue(const std::string & value);

      /// @brief direct access for debugging
      void setInitialValue(INTEGER_TYPE initialValue)
      {
        typedValue_ = initialValue;
        initialField_.reset(new FIELD_CLASS(typedValue_));
      }

      virtual void setDefaultValueIncrement()
      {
        typedValue_ = INTEGER_TYPE(1);
        initialField_ = FIELD_CLASS::create(typedValue_);
      }

      virtual bool decodeNop(
        Codecs::DataSource & source,
        Codecs::PresenceMap & pmap,
        Codecs::Decoder & decoder,
        Messages::FieldSet & fieldSet) const;

      virtual bool decodeConstant(
        Codecs::DataSource & source,
        Codecs::PresenceMap & pmap,
        Codecs::Decoder & decoder,
        Messages::FieldSet & fieldSet) const;

      virtual bool decodeDefault(
        Codecs::DataSource & source,
        Codecs::PresenceMap & pmap,
        Codecs::Decoder & decoder,
        Messages::FieldSet & fieldSet) const;

      virtual bool decodeCopy(
        Codecs::DataSource & source,
        Codecs::PresenceMap & pmap,
        Codecs::Decoder & decoder,
        Messages::FieldSet & fieldSet) const;

      virtual bool decodeDelta(
        Codecs::DataSource & source,
        Codecs::PresenceMap & pmap,
        Codecs::Decoder & decoder,
        Messages::FieldSet & fieldSet) const;

      virtual bool decodeIncrement(
        Codecs::DataSource & source,
        Codecs::PresenceMap & pmap,
        Codecs::Decoder & decoder,
        Messages::FieldSet & fieldSet) const;

      virtual void encodeNop(
        Codecs::DataDestination & destination,
        Codecs::PresenceMap & pmap,
        Codecs::Encoder & encoder,
        const Messages::FieldSet & fieldSet) const;

      virtual void encodeConstant(
        Codecs::DataDestination & destination,
        Codecs::PresenceMap & pmap,
        Codecs::Encoder & encoder,
        const Messages::FieldSet & fieldSet) const;

      virtual void encodeDefault(
        Codecs::DataDestination & destination,
        Codecs::PresenceMap & pmap,
        Codecs::Encoder & encoder,
        const Messages::FieldSet & fieldSet) const;

      virtual void encodeCopy(
        Codecs::DataDestination & destination,
        Codecs::PresenceMap & pmap,
        Codecs::Encoder & encoder,
        const Messages::FieldSet & fieldSet) const;

      virtual void encodeDelta(
        Codecs::DataDestination & destination,
        Codecs::PresenceMap & pmap,
        Codecs::Encoder & encoder,
        const Messages::FieldSet & fieldSet) const;

      virtual void encodeIncrement(
        Codecs::DataDestination & destination,
        Codecs::PresenceMap & pmap,
        Codecs::Encoder & encoder,
        const Messages::FieldSet & fieldSet) const;

    private:
      FieldInstructionInteger(const FieldInstructionInteger<INTEGER_TYPE, FIELD_CLASS, SIGNED> &);
      FieldInstructionInteger<INTEGER_TYPE, FIELD_CLASS, SIGNED> & operator=(const FieldInstructionInteger<INTEGER_TYPE, FIELD_CLASS, SIGNED> &);

    private:
      INTEGER_TYPE typedValue_;
      Messages::FieldCPtr initialField_;
    };

    template<typename INTEGER_TYPE, typename FIELD_CLASS, bool SIGNED>
    FieldInstructionInteger<INTEGER_TYPE, FIELD_CLASS, SIGNED>::
    FieldInstructionInteger(
          const std::string & name,
          const std::string & fieldNamespace)
      : FieldInstruction(name, fieldNamespace)
      , typedValue_(INTEGER_TYPE(0))
      , initialField_(FIELD_CLASS::create(0))
    {
    }

    template<typename INTEGER_TYPE, typename FIELD_CLASS, bool SIGNED>
    FieldInstructionInteger<INTEGER_TYPE, FIELD_CLASS, SIGNED>::
    FieldInstructionInteger()
      : typedValue_(INTEGER_TYPE(0))
      , initialField_(FIELD_CLASS::create(0))
    {
    }

    template<typename INTEGER_TYPE, typename FIELD_CLASS, bool SIGNED>
    FieldInstructionInteger<INTEGER_TYPE, FIELD_CLASS, SIGNED>::
    ~FieldInstructionInteger()
    {
    }

    template<typename INTEGER_TYPE, typename FIELD_CLASS, bool SIGNED>
    void
    FieldInstructionInteger<INTEGER_TYPE, FIELD_CLASS, SIGNED>::
    interpretValue(const std::string & value)
    {
      if(value.empty())
      {
        typedValue_ = INTEGER_TYPE(0);
      }
      else
      {
        typedValue_ = boost::lexical_cast<INTEGER_TYPE>(value);
      }
      initialField_ = FIELD_CLASS::create(typedValue_);
    }

    template<typename INTEGER_TYPE, typename FIELD_CLASS, bool SIGNED>
    bool
    FieldInstructionInteger<INTEGER_TYPE, FIELD_CLASS, SIGNED>::
    decodeNop(
      Codecs::DataSource & source,
      Codecs::PresenceMap & pmap,
      Codecs::Decoder & decoder,
      Messages::FieldSet & fieldSet) const
    {
      PROFILE_POINT("int::decodeNop");

      // note NOP never uses pmap.  It uses a null value instead for optional fields
      // so it's always safe to do the basic decode.
      INTEGER_TYPE value = 0;
      if(SIGNED) // expect compile-time optimization here
      {
        decodeSignedInteger(source, decoder, value);
      }
      else
      {
        decodeUnsignedInteger(source, decoder, value);
      }
      if(isMandatory())
      {
        Messages::FieldCPtr newField(FIELD_CLASS::create(value));

        fieldSet.addField(
          identity_,
          newField);
      }
      else
      {
        // not mandatory means it's nullable
        if(!checkNullInteger(value))
        {
          Messages::FieldCPtr newField(FIELD_CLASS::create(value));
          fieldSet.addField(
            identity_,
            newField);
        }
      }
      return true;
    }

    template<typename INTEGER_TYPE, typename FIELD_CLASS, bool SIGNED>
    bool
    FieldInstructionInteger<INTEGER_TYPE, FIELD_CLASS, SIGNED>::
    decodeConstant(
      Codecs::DataSource & source,
      Codecs::PresenceMap & pmap,
      Codecs::Decoder & decoder,
      Messages::FieldSet & fieldSet) const
    {
      PROFILE_POINT("int::decodeConstant");
      if(!isMandatory() && !pmap.checkNextField())
      {
        // nothing to say
      }
      else
      {
        fieldSet.addField(
          identity_,
          initialField_);
      }
      return true;
    }

    template<typename INTEGER_TYPE, typename FIELD_CLASS, bool SIGNED>
    bool
    FieldInstructionInteger<INTEGER_TYPE, FIELD_CLASS, SIGNED>::
    decodeCopy(
      Codecs::DataSource & source,
      Codecs::PresenceMap & pmap,
      Codecs::Decoder & decoder,
      Messages::FieldSet & fieldSet) const
    {
      PROFILE_POINT("int::decodeCopy");
      DictionaryPtr dictionary;
      getDictionary(decoder, dictionary);

      if(pmap.checkNextField())
      {
        INTEGER_TYPE value = typedValue_;
        // present in stream
        if(SIGNED) // expect compile-time optimization here
        {
          decodeSignedInteger(source, decoder, value);
        }
        else
        {
          decodeUnsignedInteger(source, decoder, value);
        }

        if(isMandatory())
        {
          Messages::FieldCPtr newField(FIELD_CLASS::create(value));
          fieldSet.addField(
            identity_,
            newField);
          dictionary->add(getKey(), newField);
        }
        else
        {
          // not mandatory means it's nullable
          if(checkNullInteger(value))
          {
            Messages::FieldCPtr newField(FIELD_CLASS::createNull());
            dictionary->add(getKey(), newField);
          }
          else
          {
            Messages::FieldCPtr newField(FIELD_CLASS::create(value));
            fieldSet.addField(
              identity_,
              newField);
            dictionary->add(getKey(), newField);
          }
        }

      }
      else // pmap says not present, use copy
      {
        Messages::FieldCPtr previousField;
        if(dictionary->find(getKey(), previousField))
        {
          if(previousField->isDefined())
          {
            if(previousField->isType(typedValue_))
            {
              fieldSet.addField(
                identity_,
                previousField);

            }
            else
            {
              throw TemplateDefinitionError("[ERR D4] Previous value type mismatch.");
            }
          }
          else // field present but not defined
          {
            if(isMandatory())
            {
              throw TemplateDefinitionError("[ERR D6] Mandatory field is missing.");
            }
          }
        }
        else
        {
          // value not found in dictionary
          // not a problem..  use initial value if it's available
          if(fieldOp_->hasValue())
          {
            fieldSet.addField(
              identity_,
              initialField_);
            dictionary->add(getKey(), initialField_);
          }
          else
          {
            if(isMandatory())
            {
              if(decoder.getStrict())
              {
                throw EncodingError("[ERR D5] Copy operator missing mandatory integer field/no initial value");
              }
              Messages::FieldCPtr newField(FIELD_CLASS::create(0));
              fieldSet.addField(
                identity_,
                newField);
              dictionary->add(getKey(), newField);
            }
          }
        }
      }
      return true;
    }

    template<typename INTEGER_TYPE, typename FIELD_CLASS, bool SIGNED>
    bool
    FieldInstructionInteger<INTEGER_TYPE, FIELD_CLASS, SIGNED>::
    decodeDefault(
      Codecs::DataSource & source,
      Codecs::PresenceMap & pmap,
      Codecs::Decoder & decoder,
      Messages::FieldSet & fieldSet) const
    {
      PROFILE_POINT("int::decodeDefault");
      if(pmap.checkNextField())
      {
        PROFILE_POINT("int::decodeDefault:present");
        INTEGER_TYPE value = 0;
        decodeSignedInteger(source, decoder, value);
        if(isMandatory())
        {
          Messages::FieldCPtr newField(FIELD_CLASS::create(value));
          fieldSet.addField(
            identity_,
            newField);
        }
        else
        {
          if(!checkNullInteger(value))
          {
            PROFILE_POINT("int::decodeDefault:,addexplicit");
            Messages::FieldCPtr newField(FIELD_CLASS::create(value));
            fieldSet.addField(
              identity_,
              newField);
          }
        }
      }
      else // field not in stream
      {
        PROFILE_POINT("int::decodeDefault:absent");
        if(fieldOp_->hasValue())
        {
          PROFILE_POINT("int::decodeDefault:adddefault");
          fieldSet.addField(
            identity_,
            initialField_);
        }
        else if(isMandatory())
        {
          throw EncodingError("[ERR D5]Mandatory default operator with no value.");
        }
      }
      return true;
    }

    template<typename INTEGER_TYPE, typename FIELD_CLASS, bool SIGNED>
    bool
    FieldInstructionInteger<INTEGER_TYPE, FIELD_CLASS, SIGNED>::
    decodeDelta(
      Codecs::DataSource & source,
      Codecs::PresenceMap & pmap,
      Codecs::Decoder & decoder,
      Messages::FieldSet & fieldSet) const
    {
      PROFILE_POINT("int::decodeDelta");
      int64 delta;
      decodeSignedInteger(source, decoder, delta, true);
      if(!isMandatory())
      {
        if(checkNullInteger(delta))
        {
          return true; // nothing in Message; no change to saved value
        }
      }
      NESTED_PROFILE_POINT(0, "int::decDelta::nonNull");
      DictionaryPtr dictionary;
      getDictionary(decoder, dictionary);

      INTEGER_TYPE value = typedValue_;
      Messages::FieldCPtr previousField;
#ifndef NO_PROFILING
      bool found;
      {PROFILE_POINT("int::decDelta::find");
      found = dictionary->find(getKey(), previousField);
      }
      if(found)
#else // NO_PROFILING
      if(dictionary->find(getKey(), previousField))
#endif // NO_PROFILING
      {
        PROFILE_POINT("int::decDelta::fromDictionary");
        if(previousField->isType(value))
        {
          previousField->getValue(value);
        }
        else
        {
          throw TemplateDefinitionError("[ERR D4] Previous value type mismatch.");
        }
      }
      value = INTEGER_TYPE(value + delta);
      Messages::FieldCPtr newField(FIELD_CLASS::create(value));

      {PROFILE_POINT("int::decDelta::add2Msg");
      fieldSet.addField(
        identity_,
        newField);
      } //PROFILE
      {PROFILE_POINT("int::decDelta::add2Dic");
      dictionary->add(getKey(), newField);
      } //PROFILE
      return true;
    }

    template<typename INTEGER_TYPE, typename FIELD_CLASS, bool SIGNED>
    bool
    FieldInstructionInteger<INTEGER_TYPE, FIELD_CLASS, SIGNED>::
    decodeIncrement(
      Codecs::DataSource & source,
      Codecs::PresenceMap & pmap,
      Codecs::Decoder & decoder,
      Messages::FieldSet & fieldSet) const
    {
      PROFILE_POINT("int::decodeIncrement");
      DictionaryPtr dictionary;
      getDictionary(decoder, dictionary);

      if(pmap.checkNextField())
      {
        //PROFILE_POINT("int::decodeIncrement::present");
        INTEGER_TYPE value = 0;
        if(SIGNED) // expect compile-time optimization here
        {
          decodeSignedInteger(source, decoder, value);
        }
        else
        {
          decodeUnsignedInteger(source, decoder, value);
        }
        if(isMandatory())
        {
          Messages::FieldCPtr newField(FIELD_CLASS::create(value));
          fieldSet.addField(
            identity_,
            newField);
          dictionary->add(getKey(), newField);
        }
        else
        {
          //PROFILE_POINT("int::decodeIncrement::optional");
          // not mandatory means it's nullable
          if(!checkNullInteger(value))
          {
            Messages::FieldCPtr newField(FIELD_CLASS::create(value));
            fieldSet.addField(
              identity_,
              newField);
            dictionary->add(getKey(), newField);
          }
        }
      }
      else
      {
        //PROFILE_POINT("int::decodeIncrement::absent");
        INTEGER_TYPE value = typedValue_;
        Messages::FieldCPtr previousField;
        if(dictionary->find(getKey(), previousField))
        {
          if(previousField->isType(value))
          {
            previousField->getValue(value);
            value += 1;
          }
          else
          {
            throw TemplateDefinitionError("[ERR D4] Previous value type mismatch.");
          }
        }
        else
        {
          if(fieldOp_->hasValue())
          {
            value = typedValue_;
          }
          else
          {
            if(isMandatory())
            {
              if(decoder.getStrict())
              {
                throw EncodingError("[ERRD5]: Missing initial value for Increment operator");
              }
              value = 0;
            }
            else
            {
              // missing value for optional field.  We're done
              return true;
            }
          }
        }
        Messages::FieldCPtr newField(FIELD_CLASS::create(value));
        fieldSet.addField(
          identity_,
          newField);
        dictionary->add(getKey(), newField);
      }
      return true;
    }

    template<typename INTEGER_TYPE, typename FIELD_CLASS, bool SIGNED>
    void
    FieldInstructionInteger<INTEGER_TYPE, FIELD_CLASS, SIGNED>::
    encodeNop(
      Codecs::DataDestination & destination,
      Codecs::PresenceMap & pmap,
      Codecs::Encoder & encoder,
      const Messages::FieldSet & fieldSet) const
    {
      // get the value from the application data
      Messages::FieldCPtr field;
      if(fieldSet.getField(identity_.name(), field))
      {
        INTEGER_TYPE value;
        field->getValue(value);
        if(!isMandatory())
        {
          if(value >= 0)
          {
            ++value;
          }
        }
        if(SIGNED)
        {
          encodeSignedInteger(destination, encoder.getWorkingBuffer(), value);
        }
        else
        {
          encodeUnsignedInteger(destination, encoder.getWorkingBuffer(), value);
        }
      }
      else // not defined in fieldset
      {
        if(isMandatory())
        {
          throw EncodingError("Missing mandatory field.");
        }
        destination.putByte(nullInteger);
      }
    }

    template<typename INTEGER_TYPE, typename FIELD_CLASS, bool SIGNED>
    void
    FieldInstructionInteger<INTEGER_TYPE, FIELD_CLASS, SIGNED>::
    encodeConstant(
      Codecs::DataDestination & destination,
      Codecs::PresenceMap & pmap,
      Codecs::Encoder & encoder,
      const Messages::FieldSet & fieldSet) const
    {
      // get the value from the application data
      Messages::FieldCPtr field;
      if(fieldSet.getField(identity_.name(), field))
      {
        INTEGER_TYPE value;
        field->getValue(value);
        if(value != typedValue_)
        {
          throw EncodingError("Constant value does not match application data.");
        }

        if(!isMandatory())
        {
          pmap.setNextField(true);
        }
      }
      else // not defined in fieldset
      {
        if(isMandatory())
        {
          throw EncodingError("Missing mandatory field.");
        }
        pmap.setNextField(false);
      }
    }

    template<typename INTEGER_TYPE, typename FIELD_CLASS, bool SIGNED>
    void
    FieldInstructionInteger<INTEGER_TYPE, FIELD_CLASS, SIGNED>::
    encodeDefault(
      Codecs::DataDestination & destination,
      Codecs::PresenceMap & pmap,
      Codecs::Encoder & encoder,
      const Messages::FieldSet & fieldSet) const
    {
      // get the value from the application data
      Messages::FieldCPtr field;
      if(fieldSet.getField(identity_.name(), field))
      {
        INTEGER_TYPE value;
        field->getValue(value);
        if(value == typedValue_)
        {
          pmap.setNextField(false); // not in stream. use default
        }
        else
        {
          pmap.setNextField(true); // != default.  Send value
          if(!isMandatory())
          {
            if(value >= 0)
            {
              ++value;
            }
          }
          if(SIGNED)
          {
            encodeSignedInteger(destination, encoder.getWorkingBuffer(), value);
          }
          else
          {
            encodeUnsignedInteger(destination, encoder.getWorkingBuffer(), value);
          }
        }
      }
      else // not defined in fieldset
      {
        if(isMandatory())
        {
          throw EncodingError("Missing mandatory field.");
        }
        if(fieldOp_->hasValue())
        {
          pmap.setNextField(true);
          destination.putByte(nullInteger);
        }
        else
        {
          pmap.setNextField(false);
        }
      }
    }

    template<typename INTEGER_TYPE, typename FIELD_CLASS, bool SIGNED>
    void
    FieldInstructionInteger<INTEGER_TYPE, FIELD_CLASS, SIGNED>::
    encodeCopy(
      Codecs::DataDestination & destination,
      Codecs::PresenceMap & pmap,
      Codecs::Encoder & encoder,
      const Messages::FieldSet & fieldSet) const
    {
      // declare a couple of variables...
      bool previousIsKnown = false;
      bool previousNotNull = false;
      INTEGER_TYPE previousValue = 0;

      // ... then initialize them from the dictionary
      DictionaryPtr dictionary;
      getDictionary(encoder, dictionary);
      Messages::FieldCPtr previousField;
      if(dictionary->find(getKey(), previousField))
      {
        if(!previousField->isType(typedValue_))
        {
          throw TemplateDefinitionError("[ERR D4] Previous value type mismatch.");
        }
        previousIsKnown = true;
        previousNotNull = previousField->isDefined();
        if(previousNotNull)
        {
          previousField->getValue(previousValue);
        }
      }

      // get the value from the application data
      Messages::FieldCPtr field;
      if(fieldSet.getField(identity_.name(), field))
      {
        INTEGER_TYPE value;
        field->getValue(value);

        if(previousIsKnown && previousValue == value)
        {
          pmap.setNextField(false); // not in stream, use copy
        }
        else
        {
          if(!isMandatory())
          {
            if(value >= 0)
            {
              ++value;
            }
          }
          pmap.setNextField(true);// value in stream
          if(SIGNED)
          {
            encodeSignedInteger(destination, encoder.getWorkingBuffer(), value);
          }
          else
          {
            encodeUnsignedInteger(destination, encoder.getWorkingBuffer(), value);
          }
          field = FIELD_CLASS::create(value);
          dictionary->add(getKey(), field);
        }
      }
      else // not defined in fieldset
      {
        if(isMandatory())
        {
          throw EncodingError("Missing mandatory field.");
        }
        if((previousIsKnown && previousNotNull)
          || !previousIsKnown)
        {
          pmap.setNextField(true);// value in stream
          destination.putByte(nullInteger);
          field = FIELD_CLASS::createNull();
          dictionary->add(getKey(), field);
        }
        else
        {
          pmap.setNextField(false);
        }
      }
    }

    template<typename INTEGER_TYPE, typename FIELD_CLASS, bool SIGNED>
    void
    FieldInstructionInteger<INTEGER_TYPE, FIELD_CLASS, SIGNED>::
    encodeDelta(
      Codecs::DataDestination & destination,
      Codecs::PresenceMap & pmap,
      Codecs::Encoder & encoder,
      const Messages::FieldSet & fieldSet) const
    {
      // declare a couple of variables...
      bool previousIsKnown = false;
      bool previousNotNull = true;
      INTEGER_TYPE previousValue = 0;

      // ... then initialize them from the dictionary
      DictionaryPtr dictionary;
      getDictionary(encoder, dictionary);
      Messages::FieldCPtr previousField;
      if(dictionary->find(getKey(), previousField))
      {
        if(!previousField->isType(typedValue_))
        {
          throw TemplateDefinitionError("[ERR D4] Previous value type mismatch.");
        }
        previousIsKnown = true;
        previousNotNull = previousField->isDefined();
        if(previousNotNull)
        {
          previousField->getValue(previousValue);
        }
      }

      // get the value from the application data
      Messages::FieldCPtr field;
      if(fieldSet.getField(identity_.name(), field))
      {
        INTEGER_TYPE value;
        field->getValue(value);
        int64 deltaValue = int64(value) - int64(previousValue);
        if(!isMandatory())
        {
          if(deltaValue >= 0)
          {
            deltaValue += 1;
          }
        }
        encodeSignedInteger(destination, encoder.getWorkingBuffer(), deltaValue);
        if(!previousIsKnown  || value != previousValue)
        {
          field = FIELD_CLASS::create(value);
          dictionary->add(getKey(), field);
        }

      }
      else // not defined in fieldset
      {
        if(isMandatory())
        {
          throw EncodingError("Missing mandatory field.");
        }
        destination.putByte(nullInteger);
      }
    }

    template<typename INTEGER_TYPE, typename FIELD_CLASS, bool SIGNED>
    void
    FieldInstructionInteger<INTEGER_TYPE, FIELD_CLASS, SIGNED>::
    encodeIncrement(
      Codecs::DataDestination & destination,
      Codecs::PresenceMap & pmap,
      Codecs::Encoder & encoder,
      const Messages::FieldSet & fieldSet) const
    {
      // declare a couple of variables...
      bool previousIsKnown = false;
      bool previousNotNull = false;
      INTEGER_TYPE previousValue = 0;

      // ... then initialize them from the dictionary
      DictionaryPtr dictionary;
      getDictionary(encoder, dictionary);
      Messages::FieldCPtr previousField;
      if(dictionary->find(getKey(), previousField))
      {
        if(!previousField->isType(typedValue_))
        {
          throw TemplateDefinitionError("[ERR D4] Previous value type mismatch.");
        }
        previousIsKnown = true;
        previousNotNull = previousField->isDefined();
        if(previousNotNull)
        {
          previousField->getValue(previousValue);
        }
      }

      // get the value from the application data
      Messages::FieldCPtr field;
      if(fieldSet.getField(identity_.name(), field))
      {
        INTEGER_TYPE value;
        field->getValue(value);
        if(previousValue + 1 == value)
        {
          pmap.setNextField(false);
        }
        else
        {
          if(!isMandatory())
          {
            if(value >= 0)
            {
              ++value;
            }
          }
          pmap.setNextField(true);
          if(SIGNED)
          {
            encodeSignedInteger(destination, encoder.getWorkingBuffer(), value);
          }
          else
          {
            encodeUnsignedInteger(destination, encoder.getWorkingBuffer(), value);
          }
        }
        field = FIELD_CLASS::create(value);
        dictionary->add(getKey(), field);
      }
      else // not defined in fieldset
      {
        if(isMandatory())
        {
          throw EncodingError("Missing mandatory field.");
        }
        pmap.setNextField(true);// value in stream
        destination.putByte(nullInteger);
        field = FIELD_CLASS::createNull();
        dictionary->add(getKey(), field);
      }
    }
  }
}
#endif // FIELDINSTRUCTIONINTEGER_H