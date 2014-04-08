# Generated by the protocol buffer compiler.  DO NOT EDIT!
# source: bayesact_message.proto

from google.protobuf import descriptor as _descriptor
from google.protobuf import message as _message
from google.protobuf import reflection as _reflection
from google.protobuf import descriptor_pb2
# @@protoc_insertion_point(imports)




DESCRIPTOR = _descriptor.FileDescriptor(
  name='bayesact_message.proto',
  package='EHwA',
  serialized_pb='\n\x16\x62\x61yesact_message.proto\x12\x04\x45HwA\"j\n\x0f\x42\x61yesactRequest\x12\x15\n\nevaluation\x18\x01 \x02(\x01:\x01\x30\x12\x12\n\x07potency\x18\x02 \x02(\x01:\x01\x30\x12\x13\n\x08\x61\x63tivity\x18\x03 \x02(\x01:\x01\x30\x12\x17\n\x0bhand_action\x18\x04 \x02(\x05:\x02-1\"f\n\x10\x42\x61yesactResponse\x12\x15\n\nevaluation\x18\x01 \x02(\x01:\x01\x30\x12\x12\n\x07potency\x18\x02 \x02(\x01:\x01\x30\x12\x13\n\x08\x61\x63tivity\x18\x03 \x02(\x01:\x01\x30\x12\x12\n\x06prompt\x18\x04 \x02(\x05:\x02-1')




_BAYESACTREQUEST = _descriptor.Descriptor(
  name='BayesactRequest',
  full_name='EHwA.BayesactRequest',
  filename=None,
  file=DESCRIPTOR,
  containing_type=None,
  fields=[
    _descriptor.FieldDescriptor(
      name='evaluation', full_name='EHwA.BayesactRequest.evaluation', index=0,
      number=1, type=1, cpp_type=5, label=2,
      has_default_value=True, default_value=0,
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      options=None),
    _descriptor.FieldDescriptor(
      name='potency', full_name='EHwA.BayesactRequest.potency', index=1,
      number=2, type=1, cpp_type=5, label=2,
      has_default_value=True, default_value=0,
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      options=None),
    _descriptor.FieldDescriptor(
      name='activity', full_name='EHwA.BayesactRequest.activity', index=2,
      number=3, type=1, cpp_type=5, label=2,
      has_default_value=True, default_value=0,
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      options=None),
    _descriptor.FieldDescriptor(
      name='hand_action', full_name='EHwA.BayesactRequest.hand_action', index=3,
      number=4, type=5, cpp_type=1, label=2,
      has_default_value=True, default_value=-1,
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      options=None),
  ],
  extensions=[
  ],
  nested_types=[],
  enum_types=[
  ],
  options=None,
  is_extendable=False,
  extension_ranges=[],
  serialized_start=32,
  serialized_end=138,
)


_BAYESACTRESPONSE = _descriptor.Descriptor(
  name='BayesactResponse',
  full_name='EHwA.BayesactResponse',
  filename=None,
  file=DESCRIPTOR,
  containing_type=None,
  fields=[
    _descriptor.FieldDescriptor(
      name='evaluation', full_name='EHwA.BayesactResponse.evaluation', index=0,
      number=1, type=1, cpp_type=5, label=2,
      has_default_value=True, default_value=0,
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      options=None),
    _descriptor.FieldDescriptor(
      name='potency', full_name='EHwA.BayesactResponse.potency', index=1,
      number=2, type=1, cpp_type=5, label=2,
      has_default_value=True, default_value=0,
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      options=None),
    _descriptor.FieldDescriptor(
      name='activity', full_name='EHwA.BayesactResponse.activity', index=2,
      number=3, type=1, cpp_type=5, label=2,
      has_default_value=True, default_value=0,
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      options=None),
    _descriptor.FieldDescriptor(
      name='prompt', full_name='EHwA.BayesactResponse.prompt', index=3,
      number=4, type=5, cpp_type=1, label=2,
      has_default_value=True, default_value=-1,
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      options=None),
  ],
  extensions=[
  ],
  nested_types=[],
  enum_types=[
  ],
  options=None,
  is_extendable=False,
  extension_ranges=[],
  serialized_start=140,
  serialized_end=242,
)

DESCRIPTOR.message_types_by_name['BayesactRequest'] = _BAYESACTREQUEST
DESCRIPTOR.message_types_by_name['BayesactResponse'] = _BAYESACTRESPONSE

class BayesactRequest(_message.Message):
  __metaclass__ = _reflection.GeneratedProtocolMessageType
  DESCRIPTOR = _BAYESACTREQUEST

  # @@protoc_insertion_point(class_scope:EHwA.BayesactRequest)

class BayesactResponse(_message.Message):
  __metaclass__ = _reflection.GeneratedProtocolMessageType
  DESCRIPTOR = _BAYESACTRESPONSE

  # @@protoc_insertion_point(class_scope:EHwA.BayesactResponse)


# @@protoc_insertion_point(module_scope)
