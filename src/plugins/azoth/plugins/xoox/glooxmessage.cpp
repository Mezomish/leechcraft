/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2011  Georg Rudoy
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#include "glooxmessage.h"
#include <QtDebug>
#include <QXmppClient.h>
#include "glooxclentry.h"
#include "clientconnection.h"
#include "core.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Xoox
{
	GlooxMessage::GlooxMessage (IMessage::MessageType type,
			IMessage::Direction dir,
			const QString& jid,
			const QString& variant,
			ClientConnection *conn)
	: Type_ (type)
	, SubType_ (MSTOther)
	, Direction_ (dir)
	, BareJID_ (jid)
	, Variant_ (variant)
	, Connection_ (conn)
	{
		const QString& remoteJid = variant.isEmpty () ?
				jid :
				jid + "/" + variant;
		if (type == MTChatMessage && variant.isEmpty ())
		{
			QObject *object = Connection_->GetCLEntry (jid, variant);
			Variant_ = qobject_cast<ICLEntry*> (object)->Variants ().value (0);
		}
		Message_.setTo (dir == DIn ? conn->GetOurJID () : remoteJid);
		Message_.setStamp (QDateTime::currentDateTime ());
	}

	GlooxMessage::GlooxMessage (const QXmppMessage& message,
			ClientConnection *conn)
	: Type_ (MTChatMessage)
	, Direction_ (DIn)
	, Message_ (message)
	, Connection_ (conn)
	{
		Connection_->Split (message.from (), &BareJID_, &Variant_);
		if (!Message_.stamp ().isValid ())
			Message_.setStamp (QDateTime::currentDateTime ());
		else
			Message_.setStamp (Message_.stamp ().toLocalTime ());
	}

	QObject* GlooxMessage::GetObject ()
	{
		return this;
	}

	void GlooxMessage::Send ()
	{
		if (Direction_ == DIn)
		{
			qWarning () << Q_FUNC_INFO
					<< "tried to send incoming message";
			return;
		}

		switch (Type_)
		{
		case MTChatMessage:
		case MTMUCMessage:
			Connection_->GetClient ()->sendPacket (Message_);
			return;
		default:
			qWarning () << Q_FUNC_INFO
					<< this
					<< "cannot send a message of type"
					<< Type_;
			break;
		}
	}

	IMessage::Direction GlooxMessage::GetDirection () const
	{
		return Direction_;
	}

	IMessage::MessageType GlooxMessage::GetMessageType () const
	{
		return Type_;
	}

	IMessage::MessageSubType GlooxMessage::GetMessageSubType () const
	{
		return SubType_;
	}
	
	void GlooxMessage::SetMessageSubType (IMessage::MessageSubType subType)
	{
		SubType_ = subType;
	}

	QObject* GlooxMessage::OtherPart () const
	{
		return Connection_->GetCLEntry (BareJID_, Variant_);
	}

	QString GlooxMessage::GetOtherVariant () const
	{
		return Variant_;
	}

	QString GlooxMessage::GetBody () const
	{
		return Message_.body ();
	}

	void GlooxMessage::SetBody (const QString& body)
	{
		Message_.setBody (body);
	}

	QDateTime GlooxMessage::GetDateTime () const
	{
		return Message_.stamp ();
	}

	void GlooxMessage::SetDateTime (const QDateTime& dateTime)
	{
		Message_.setStamp (dateTime);
	}
}
}
}
