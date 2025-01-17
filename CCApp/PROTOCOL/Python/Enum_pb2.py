# -*- coding: utf-8 -*-
# Generated by the protocol buffer compiler.  DO NOT EDIT!
# source: Enum.proto

from google.protobuf.internal import enum_type_wrapper
from google.protobuf import descriptor as _descriptor
from google.protobuf import message as _message
from google.protobuf import reflection as _reflection
from google.protobuf import symbol_database as _symbol_database
# @@protoc_insertion_point(imports)

_sym_db = _symbol_database.Default()




DESCRIPTOR = _descriptor.FileDescriptor(
  name='Enum.proto',
  package='',
  syntax='proto3',
  serialized_options=b'H\001',
  serialized_pb=b'\n\nEnum.proto*\xba\r\n\x08PROTOCOL\x12\x16\n\x12ID_PKT_VERSION_REQ\x10\x00\x12\x16\n\x12ID_PKT_VERSION_RES\x10\x01\x12\x14\n\x10ID_PKT_LOGIN_REQ\x10\x02\x12\x14\n\x10ID_PKT_LOGIN_RES\x10\x03\x12\x1a\n\x16ID_PKT_CREATE_ROOM_REQ\x10\x04\x12\x1a\n\x16ID_PKT_CREATE_ROOM_RES\x10\x05\x12\x19\n\x15ID_PKT_ENTER_ROOM_REQ\x10\x06\x12\x19\n\x15ID_PKT_ENTER_ROOM_RES\x10\x07\x12\x19\n\x15ID_PKT_LEAVE_ROOM_REQ\x10\x08\x12\x19\n\x15ID_PKT_LEAVE_ROOM_RES\x10\t\x12\x1d\n\x19ID_PKT_BITMAP_MESSAGE_REQ\x10\n\x12\x1d\n\x19ID_PKT_BITMAP_MESSAGE_RES\x10\x0b\x12\x1f\n\x1bID_PKT_NEW_USER_IN_ROOM_NTY\x10\x0c\x12\x18\n\x14ID_PKT_ROOM_LIST_REQ\x10\r\x12\x18\n\x14ID_PKT_ROOM_LIST_RES\x10\x0e\x12\x19\n\x15ID_PKT_GAME_START_NTY\x10\x0f\x12\x1a\n\x16ID_PKT_GAME_RESULT_NTY\x10\x10\x12 \n\x1cID_PKT_ROOM_PASS_THROUGH_REQ\x10\x11\x12 \n\x1cID_PKT_ROOM_PASS_THROUGH_RES\x10\x12\x12\x15\n\x11ID_PKT_NOTICE_REQ\x10\x13\x12\x15\n\x11ID_PKT_NOTICE_RES\x10\x14\x12\x1c\n\x18ID_PKT_AUDIO_MESSAGE_REQ\x10\x15\x12\x1c\n\x18ID_PKT_AUDIO_MESSAGE_RES\x10\x16\x12\x1b\n\x17ID_PKT_PRAY_MESSAGE_REQ\x10\x17\x12\x1b\n\x17ID_PKT_PRAY_MESSAGE_RES\x10\x18\x12\x1f\n\x1bID_PKT_PRAY_MESSAGE_REG_REQ\x10\x19\x12\x1f\n\x1bID_PKT_PRAY_MESSAGE_REG_RES\x10\x1a\x12\x12\n\x0eID_PKT_QNA_REQ\x10\x1b\x12\x12\n\x0eID_PKT_QNA_RES\x10\x1c\x12\x18\n\x14ID_PKT_MAIL_LIST_REQ\x10\x1d\x12\x18\n\x14ID_PKT_MAIL_LIST_RES\x10\x1e\x12\x18\n\x14ID_PKT_MAIL_SEND_REQ\x10\x1f\x12\x18\n\x14ID_PKT_MAIL_SEND_RES\x10 \x12\x17\n\x13ID_PKT_MAIL_DEL_REQ\x10!\x12\x17\n\x13ID_PKT_MAIL_DEL_RES\x10\"\x12\x18\n\x14ID_PKT_MAIL_READ_REQ\x10#\x12\x18\n\x14ID_PKT_MAIL_READ_RES\x10$\x12\x12\n\x0eID_PKT_REG_REQ\x10%\x12\x12\n\x0eID_PKT_REG_RES\x10&\x12\x1c\n\x18ID_PKT_CLIENT_LOGOUT_REQ\x10\'\x12\x1c\n\x18ID_PKT_CLIENT_LOGOUT_RES\x10(\x12\x16\n\x12ID_PKT_CLIENT_KICK\x10)\x12\x1d\n\x19ID_PKT_CAMERA_CONTROL_REQ\x10*\x12\x1d\n\x19ID_PKT_CAMERA_CONTROL_RES\x10+\x12\x1e\n\x1aID_PKT_MPEG2TS_MESSAGE_REQ\x10,\x12\x1e\n\x1aID_PKT_MPEG2TS_MESSAGE_RES\x10-\x12\x1d\n\x19ID_PKT_CAMERA_WAKE_UP_REQ\x10.\x12\x1d\n\x19ID_PKT_CAMERA_WAKE_UP_RES\x10/\x12\x1a\n\x16ID_PKT_STOP_STREAM_REQ\x10\x30\x12\x1a\n\x16ID_PKT_STOP_STREAM_RES\x10\x31\x12\x19\n\x15ID_PKT_REG_CAMERA_REQ\x10\x32\x12\x19\n\x15ID_PKT_REG_CAMERA_RES\x10\x33\x12\x1a\n\x16ID_PKT_CAMERA_LIST_REQ\x10\x34\x12\x1a\n\x16ID_PKT_CAMERA_LIST_RES\x10\x35\x12\x1d\n\x19ID_PKT_MACHINE_STATUS_REQ\x10\x36\x12\x1d\n\x19ID_PKT_MACHINE_STATUS_RES\x10\x37\x12\x1e\n\x1aID_PKT_VERIFY_PURCHASE_REQ\x10\x38\x12\x1e\n\x1aID_PKT_VERIFY_PURCHASE_RES\x10\x39\x12\x19\n\x15ID_PKT_DEL_CAMERA_REQ\x10:\x12\x19\n\x15ID_PKT_DEL_CAMERA_RES\x10;\x12\x18\n\x14ID_PKT_RECONNECT_REQ\x10<\x12\x18\n\x14ID_PKT_RECONNECT_RES\x10=*{\n\tErrorCode\x12\x0b\n\x07Success\x10\x00\x12\x11\n\rDataBaseError\x10\x01\x12\x12\n\x0eLoginDuplicate\x10\x02\x12\x0f\n\x0bSystemError\x10\x03\x12\x0f\n\x0bLoginFailed\x10\x04\x12\x18\n\x14\x44uplicate_Enter_Room\x10\x05*g\n\x11\x43\x61meraControlType\x12\x10\n\x0cSwitchCamera\x10\x00\x12\r\n\tQualityUp\x10\x01\x12\x0f\n\x0bQualityDown\x10\x02\x12\t\n\x05\x46lash\x10\x03\x12\x0c\n\x08SaveFile\x10\x04\x12\x07\n\x03MIC\x10\x05\x42\x02H\x01\x62\x06proto3'
)

_PROTOCOL = _descriptor.EnumDescriptor(
  name='PROTOCOL',
  full_name='PROTOCOL',
  filename=None,
  file=DESCRIPTOR,
  values=[
    _descriptor.EnumValueDescriptor(
      name='ID_PKT_VERSION_REQ', index=0, number=0,
      serialized_options=None,
      type=None),
    _descriptor.EnumValueDescriptor(
      name='ID_PKT_VERSION_RES', index=1, number=1,
      serialized_options=None,
      type=None),
    _descriptor.EnumValueDescriptor(
      name='ID_PKT_LOGIN_REQ', index=2, number=2,
      serialized_options=None,
      type=None),
    _descriptor.EnumValueDescriptor(
      name='ID_PKT_LOGIN_RES', index=3, number=3,
      serialized_options=None,
      type=None),
    _descriptor.EnumValueDescriptor(
      name='ID_PKT_CREATE_ROOM_REQ', index=4, number=4,
      serialized_options=None,
      type=None),
    _descriptor.EnumValueDescriptor(
      name='ID_PKT_CREATE_ROOM_RES', index=5, number=5,
      serialized_options=None,
      type=None),
    _descriptor.EnumValueDescriptor(
      name='ID_PKT_ENTER_ROOM_REQ', index=6, number=6,
      serialized_options=None,
      type=None),
    _descriptor.EnumValueDescriptor(
      name='ID_PKT_ENTER_ROOM_RES', index=7, number=7,
      serialized_options=None,
      type=None),
    _descriptor.EnumValueDescriptor(
      name='ID_PKT_LEAVE_ROOM_REQ', index=8, number=8,
      serialized_options=None,
      type=None),
    _descriptor.EnumValueDescriptor(
      name='ID_PKT_LEAVE_ROOM_RES', index=9, number=9,
      serialized_options=None,
      type=None),
    _descriptor.EnumValueDescriptor(
      name='ID_PKT_BITMAP_MESSAGE_REQ', index=10, number=10,
      serialized_options=None,
      type=None),
    _descriptor.EnumValueDescriptor(
      name='ID_PKT_BITMAP_MESSAGE_RES', index=11, number=11,
      serialized_options=None,
      type=None),
    _descriptor.EnumValueDescriptor(
      name='ID_PKT_NEW_USER_IN_ROOM_NTY', index=12, number=12,
      serialized_options=None,
      type=None),
    _descriptor.EnumValueDescriptor(
      name='ID_PKT_ROOM_LIST_REQ', index=13, number=13,
      serialized_options=None,
      type=None),
    _descriptor.EnumValueDescriptor(
      name='ID_PKT_ROOM_LIST_RES', index=14, number=14,
      serialized_options=None,
      type=None),
    _descriptor.EnumValueDescriptor(
      name='ID_PKT_GAME_START_NTY', index=15, number=15,
      serialized_options=None,
      type=None),
    _descriptor.EnumValueDescriptor(
      name='ID_PKT_GAME_RESULT_NTY', index=16, number=16,
      serialized_options=None,
      type=None),
    _descriptor.EnumValueDescriptor(
      name='ID_PKT_ROOM_PASS_THROUGH_REQ', index=17, number=17,
      serialized_options=None,
      type=None),
    _descriptor.EnumValueDescriptor(
      name='ID_PKT_ROOM_PASS_THROUGH_RES', index=18, number=18,
      serialized_options=None,
      type=None),
    _descriptor.EnumValueDescriptor(
      name='ID_PKT_NOTICE_REQ', index=19, number=19,
      serialized_options=None,
      type=None),
    _descriptor.EnumValueDescriptor(
      name='ID_PKT_NOTICE_RES', index=20, number=20,
      serialized_options=None,
      type=None),
    _descriptor.EnumValueDescriptor(
      name='ID_PKT_AUDIO_MESSAGE_REQ', index=21, number=21,
      serialized_options=None,
      type=None),
    _descriptor.EnumValueDescriptor(
      name='ID_PKT_AUDIO_MESSAGE_RES', index=22, number=22,
      serialized_options=None,
      type=None),
    _descriptor.EnumValueDescriptor(
      name='ID_PKT_PRAY_MESSAGE_REQ', index=23, number=23,
      serialized_options=None,
      type=None),
    _descriptor.EnumValueDescriptor(
      name='ID_PKT_PRAY_MESSAGE_RES', index=24, number=24,
      serialized_options=None,
      type=None),
    _descriptor.EnumValueDescriptor(
      name='ID_PKT_PRAY_MESSAGE_REG_REQ', index=25, number=25,
      serialized_options=None,
      type=None),
    _descriptor.EnumValueDescriptor(
      name='ID_PKT_PRAY_MESSAGE_REG_RES', index=26, number=26,
      serialized_options=None,
      type=None),
    _descriptor.EnumValueDescriptor(
      name='ID_PKT_QNA_REQ', index=27, number=27,
      serialized_options=None,
      type=None),
    _descriptor.EnumValueDescriptor(
      name='ID_PKT_QNA_RES', index=28, number=28,
      serialized_options=None,
      type=None),
    _descriptor.EnumValueDescriptor(
      name='ID_PKT_MAIL_LIST_REQ', index=29, number=29,
      serialized_options=None,
      type=None),
    _descriptor.EnumValueDescriptor(
      name='ID_PKT_MAIL_LIST_RES', index=30, number=30,
      serialized_options=None,
      type=None),
    _descriptor.EnumValueDescriptor(
      name='ID_PKT_MAIL_SEND_REQ', index=31, number=31,
      serialized_options=None,
      type=None),
    _descriptor.EnumValueDescriptor(
      name='ID_PKT_MAIL_SEND_RES', index=32, number=32,
      serialized_options=None,
      type=None),
    _descriptor.EnumValueDescriptor(
      name='ID_PKT_MAIL_DEL_REQ', index=33, number=33,
      serialized_options=None,
      type=None),
    _descriptor.EnumValueDescriptor(
      name='ID_PKT_MAIL_DEL_RES', index=34, number=34,
      serialized_options=None,
      type=None),
    _descriptor.EnumValueDescriptor(
      name='ID_PKT_MAIL_READ_REQ', index=35, number=35,
      serialized_options=None,
      type=None),
    _descriptor.EnumValueDescriptor(
      name='ID_PKT_MAIL_READ_RES', index=36, number=36,
      serialized_options=None,
      type=None),
    _descriptor.EnumValueDescriptor(
      name='ID_PKT_REG_REQ', index=37, number=37,
      serialized_options=None,
      type=None),
    _descriptor.EnumValueDescriptor(
      name='ID_PKT_REG_RES', index=38, number=38,
      serialized_options=None,
      type=None),
    _descriptor.EnumValueDescriptor(
      name='ID_PKT_CLIENT_LOGOUT_REQ', index=39, number=39,
      serialized_options=None,
      type=None),
    _descriptor.EnumValueDescriptor(
      name='ID_PKT_CLIENT_LOGOUT_RES', index=40, number=40,
      serialized_options=None,
      type=None),
    _descriptor.EnumValueDescriptor(
      name='ID_PKT_CLIENT_KICK', index=41, number=41,
      serialized_options=None,
      type=None),
    _descriptor.EnumValueDescriptor(
      name='ID_PKT_CAMERA_CONTROL_REQ', index=42, number=42,
      serialized_options=None,
      type=None),
    _descriptor.EnumValueDescriptor(
      name='ID_PKT_CAMERA_CONTROL_RES', index=43, number=43,
      serialized_options=None,
      type=None),
    _descriptor.EnumValueDescriptor(
      name='ID_PKT_MPEG2TS_MESSAGE_REQ', index=44, number=44,
      serialized_options=None,
      type=None),
    _descriptor.EnumValueDescriptor(
      name='ID_PKT_MPEG2TS_MESSAGE_RES', index=45, number=45,
      serialized_options=None,
      type=None),
    _descriptor.EnumValueDescriptor(
      name='ID_PKT_CAMERA_WAKE_UP_REQ', index=46, number=46,
      serialized_options=None,
      type=None),
    _descriptor.EnumValueDescriptor(
      name='ID_PKT_CAMERA_WAKE_UP_RES', index=47, number=47,
      serialized_options=None,
      type=None),
    _descriptor.EnumValueDescriptor(
      name='ID_PKT_STOP_STREAM_REQ', index=48, number=48,
      serialized_options=None,
      type=None),
    _descriptor.EnumValueDescriptor(
      name='ID_PKT_STOP_STREAM_RES', index=49, number=49,
      serialized_options=None,
      type=None),
    _descriptor.EnumValueDescriptor(
      name='ID_PKT_REG_CAMERA_REQ', index=50, number=50,
      serialized_options=None,
      type=None),
    _descriptor.EnumValueDescriptor(
      name='ID_PKT_REG_CAMERA_RES', index=51, number=51,
      serialized_options=None,
      type=None),
    _descriptor.EnumValueDescriptor(
      name='ID_PKT_CAMERA_LIST_REQ', index=52, number=52,
      serialized_options=None,
      type=None),
    _descriptor.EnumValueDescriptor(
      name='ID_PKT_CAMERA_LIST_RES', index=53, number=53,
      serialized_options=None,
      type=None),
    _descriptor.EnumValueDescriptor(
      name='ID_PKT_MACHINE_STATUS_REQ', index=54, number=54,
      serialized_options=None,
      type=None),
    _descriptor.EnumValueDescriptor(
      name='ID_PKT_MACHINE_STATUS_RES', index=55, number=55,
      serialized_options=None,
      type=None),
    _descriptor.EnumValueDescriptor(
      name='ID_PKT_VERIFY_PURCHASE_REQ', index=56, number=56,
      serialized_options=None,
      type=None),
    _descriptor.EnumValueDescriptor(
      name='ID_PKT_VERIFY_PURCHASE_RES', index=57, number=57,
      serialized_options=None,
      type=None),
    _descriptor.EnumValueDescriptor(
      name='ID_PKT_DEL_CAMERA_REQ', index=58, number=58,
      serialized_options=None,
      type=None),
    _descriptor.EnumValueDescriptor(
      name='ID_PKT_DEL_CAMERA_RES', index=59, number=59,
      serialized_options=None,
      type=None),
    _descriptor.EnumValueDescriptor(
      name='ID_PKT_RECONNECT_REQ', index=60, number=60,
      serialized_options=None,
      type=None),
    _descriptor.EnumValueDescriptor(
      name='ID_PKT_RECONNECT_RES', index=61, number=61,
      serialized_options=None,
      type=None),
  ],
  containing_type=None,
  serialized_options=None,
  serialized_start=15,
  serialized_end=1737,
)
_sym_db.RegisterEnumDescriptor(_PROTOCOL)

PROTOCOL = enum_type_wrapper.EnumTypeWrapper(_PROTOCOL)
_ERRORCODE = _descriptor.EnumDescriptor(
  name='ErrorCode',
  full_name='ErrorCode',
  filename=None,
  file=DESCRIPTOR,
  values=[
    _descriptor.EnumValueDescriptor(
      name='Success', index=0, number=0,
      serialized_options=None,
      type=None),
    _descriptor.EnumValueDescriptor(
      name='DataBaseError', index=1, number=1,
      serialized_options=None,
      type=None),
    _descriptor.EnumValueDescriptor(
      name='LoginDuplicate', index=2, number=2,
      serialized_options=None,
      type=None),
    _descriptor.EnumValueDescriptor(
      name='SystemError', index=3, number=3,
      serialized_options=None,
      type=None),
    _descriptor.EnumValueDescriptor(
      name='LoginFailed', index=4, number=4,
      serialized_options=None,
      type=None),
    _descriptor.EnumValueDescriptor(
      name='Duplicate_Enter_Room', index=5, number=5,
      serialized_options=None,
      type=None),
  ],
  containing_type=None,
  serialized_options=None,
  serialized_start=1739,
  serialized_end=1862,
)
_sym_db.RegisterEnumDescriptor(_ERRORCODE)

ErrorCode = enum_type_wrapper.EnumTypeWrapper(_ERRORCODE)
_CAMERACONTROLTYPE = _descriptor.EnumDescriptor(
  name='CameraControlType',
  full_name='CameraControlType',
  filename=None,
  file=DESCRIPTOR,
  values=[
    _descriptor.EnumValueDescriptor(
      name='SwitchCamera', index=0, number=0,
      serialized_options=None,
      type=None),
    _descriptor.EnumValueDescriptor(
      name='QualityUp', index=1, number=1,
      serialized_options=None,
      type=None),
    _descriptor.EnumValueDescriptor(
      name='QualityDown', index=2, number=2,
      serialized_options=None,
      type=None),
    _descriptor.EnumValueDescriptor(
      name='Flash', index=3, number=3,
      serialized_options=None,
      type=None),
    _descriptor.EnumValueDescriptor(
      name='SaveFile', index=4, number=4,
      serialized_options=None,
      type=None),
    _descriptor.EnumValueDescriptor(
      name='MIC', index=5, number=5,
      serialized_options=None,
      type=None),
  ],
  containing_type=None,
  serialized_options=None,
  serialized_start=1864,
  serialized_end=1967,
)
_sym_db.RegisterEnumDescriptor(_CAMERACONTROLTYPE)

CameraControlType = enum_type_wrapper.EnumTypeWrapper(_CAMERACONTROLTYPE)
ID_PKT_VERSION_REQ = 0
ID_PKT_VERSION_RES = 1
ID_PKT_LOGIN_REQ = 2
ID_PKT_LOGIN_RES = 3
ID_PKT_CREATE_ROOM_REQ = 4
ID_PKT_CREATE_ROOM_RES = 5
ID_PKT_ENTER_ROOM_REQ = 6
ID_PKT_ENTER_ROOM_RES = 7
ID_PKT_LEAVE_ROOM_REQ = 8
ID_PKT_LEAVE_ROOM_RES = 9
ID_PKT_BITMAP_MESSAGE_REQ = 10
ID_PKT_BITMAP_MESSAGE_RES = 11
ID_PKT_NEW_USER_IN_ROOM_NTY = 12
ID_PKT_ROOM_LIST_REQ = 13
ID_PKT_ROOM_LIST_RES = 14
ID_PKT_GAME_START_NTY = 15
ID_PKT_GAME_RESULT_NTY = 16
ID_PKT_ROOM_PASS_THROUGH_REQ = 17
ID_PKT_ROOM_PASS_THROUGH_RES = 18
ID_PKT_NOTICE_REQ = 19
ID_PKT_NOTICE_RES = 20
ID_PKT_AUDIO_MESSAGE_REQ = 21
ID_PKT_AUDIO_MESSAGE_RES = 22
ID_PKT_PRAY_MESSAGE_REQ = 23
ID_PKT_PRAY_MESSAGE_RES = 24
ID_PKT_PRAY_MESSAGE_REG_REQ = 25
ID_PKT_PRAY_MESSAGE_REG_RES = 26
ID_PKT_QNA_REQ = 27
ID_PKT_QNA_RES = 28
ID_PKT_MAIL_LIST_REQ = 29
ID_PKT_MAIL_LIST_RES = 30
ID_PKT_MAIL_SEND_REQ = 31
ID_PKT_MAIL_SEND_RES = 32
ID_PKT_MAIL_DEL_REQ = 33
ID_PKT_MAIL_DEL_RES = 34
ID_PKT_MAIL_READ_REQ = 35
ID_PKT_MAIL_READ_RES = 36
ID_PKT_REG_REQ = 37
ID_PKT_REG_RES = 38
ID_PKT_CLIENT_LOGOUT_REQ = 39
ID_PKT_CLIENT_LOGOUT_RES = 40
ID_PKT_CLIENT_KICK = 41
ID_PKT_CAMERA_CONTROL_REQ = 42
ID_PKT_CAMERA_CONTROL_RES = 43
ID_PKT_MPEG2TS_MESSAGE_REQ = 44
ID_PKT_MPEG2TS_MESSAGE_RES = 45
ID_PKT_CAMERA_WAKE_UP_REQ = 46
ID_PKT_CAMERA_WAKE_UP_RES = 47
ID_PKT_STOP_STREAM_REQ = 48
ID_PKT_STOP_STREAM_RES = 49
ID_PKT_REG_CAMERA_REQ = 50
ID_PKT_REG_CAMERA_RES = 51
ID_PKT_CAMERA_LIST_REQ = 52
ID_PKT_CAMERA_LIST_RES = 53
ID_PKT_MACHINE_STATUS_REQ = 54
ID_PKT_MACHINE_STATUS_RES = 55
ID_PKT_VERIFY_PURCHASE_REQ = 56
ID_PKT_VERIFY_PURCHASE_RES = 57
ID_PKT_DEL_CAMERA_REQ = 58
ID_PKT_DEL_CAMERA_RES = 59
ID_PKT_RECONNECT_REQ = 60
ID_PKT_RECONNECT_RES = 61
Success = 0
DataBaseError = 1
LoginDuplicate = 2
SystemError = 3
LoginFailed = 4
Duplicate_Enter_Room = 5
SwitchCamera = 0
QualityUp = 1
QualityDown = 2
Flash = 3
SaveFile = 4
MIC = 5


DESCRIPTOR.enum_types_by_name['PROTOCOL'] = _PROTOCOL
DESCRIPTOR.enum_types_by_name['ErrorCode'] = _ERRORCODE
DESCRIPTOR.enum_types_by_name['CameraControlType'] = _CAMERACONTROLTYPE
_sym_db.RegisterFileDescriptor(DESCRIPTOR)


DESCRIPTOR._options = None
# @@protoc_insertion_point(module_scope)
