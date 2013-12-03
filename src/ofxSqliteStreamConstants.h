#pragma once

#define OFX_SQLITE_STREAM_BEGIN_NAMESPACE namespace ofx { namespace SqliteStream {
#define OFX_SQLITE_STREAM_END_NAMESPACE } }

OFX_SQLITE_STREAM_BEGIN_NAMESPACE

typedef float TimeStamp;

OFX_SQLITE_STREAM_END_NAMESPACE

namespace ofxSqliteStream = ofx::SqliteStream;