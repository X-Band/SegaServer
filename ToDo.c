/*


__________________________________________________________________________________________
Done:

Have acks and shit done, need to get them working properly.  Find all the places where the sender
thinks he knows how much he can send, fix them not to send any more.

When the sender thinks the receiver
is out of space, have him set the ack bit to get an acknowledge so he can update the values and
send more.  The sender sees the ack come back and somehow knows what to do to get thing pumping 
again.

Write side async stuff is a sequence of FifoWrite with the actual data, FifoPeek to send it, and
FifoSkip when the data is acked.  That way we can get the data back out when we need it for
retrans.
This needs to be written like the read side with async completion routines to read from the
write buffer and put it on the line.  The difference is there is a finite amount of write space
on async writes if there is data in the write fifo.  If there is data waiting on the write fifo
and an async write comes in, the data must fit in the remaining write fifo space.  If it doesn't
fit, we spin on OSIdle/NetIdle until the data is flushed.  If the data doesn't fit in the fifo,
we have to wait until the fifo is empty then swap in the new fifo.  This should never happen
though.
This whole thing has to be layered so the data that is being sent goes into a session fifo,
then packetized and written to the phys layer as it is ready.  What ought to happen here on the
async write call is all the data is written to the session fifo in order as raw data and a packet
put together.  This is sent down to PWritePacketAsync, which puts it in the outgoing byte fifo
and the interrupt guy sends it from there.  This can happen for as much data as we have in
the fifo, up to the amount that the reciever can handle.  At the point that the reciever is
out of space, we return from the async call and TNetIdle will watch for TIndication to report
that the reciever got it all.  TNetIdle will then send some more by calling the async write
guy.
The sender doesn't know how much space the reciever has until they transact once, which means
an opening dialog or something.  Otherwise neither will send because they don't think there is
space.

Something amiss with the framing values?  Look first at the packet send sequence..

Finish the client fifo stuff

Step Spy the damaged buffer...  from FifoUnwrite.  Make sure that it works, it probably doesn't!

Why is the ack not flushing the buffer?

Get connection establishment dialog up.  This should start up as something that always denys
connections, so we can see that it works.  Next, there is the actual connection.  Looking
at the current architecture, it is probably going to be best to have it set up that TIndication
just modifies the session state variables, and TOpen basically just polls it and calls TNetIdle
to find out what is going on.  TOpen doesn't really care what the session got set up as, it 
just wants to make sure that it was done.  Polling off the state variable should accomplish
this fine.

Get CRC into phys layer.  This needs to return the value instead of just returning a boolean.
This can then be used on the write side on the buffer that is already queued up, then we
can calc the CRC as we FifoWrite.

TListen seems to be up and denying requests.  Need to bring it up so it can do the three way
thing.  Three way thing is done, put rest of unimplemented control calls: CloseConnAdv, FwdReset,
and RetransAdv.  Retrans may be already implemented as a control send with the relevant info.
Close is done.

Need to make sure that the transitive states (i.e. while waiting for an ack on a forward reset)
resend the correct packet if the packet gets lost.  The wrong thing is just to send tickles after
the timeout.

Work on Serial code.  We have a 16 byte fifo on reads and writes for both game play and server
communication.  This is a good thing.  Need to use it to feed our fifos.

__________________________________________________________________________________________
To Do:

Bitties:
	Closure on async listen problem.  It seems that the best way to do this is a combination of
	resends on the open dialog and not calling PNetIdle until the session is set up.  This way, if
	the link _is_ closed, we are assured that there is no chance of a packet coming in anyways
	and we can tear it down after we sense that.  This is changed, just need to verify on simulator
	and check in.
	
	Get resends going on the open dialog.
	
	Figure what is with the expoential time resend screwup.  Lower the resend time to 5 secs.
	
	Bring things up again on Sega.  Need to merge changes from gametalk, etc into the other phys layers.
	
	Finish game-game, test it on sega.
	
	Finish NetIdle pblock.
	
	Timeouts on reads and writes.
	
	NetIdleUntilFlushed call.  This should also timeout as normal.

Work on Game Protocol.  We still have a 16 byte fifo, but the fifo doesn't work in sync mode.
	If we did a partitioning scheme on the protocol discriminator, we could say that the high but set
	is a game frame.  The rest of the byte could be used then for other things.  We don't have to
	worry about obsoleting this protocol because higher speed modems will probably be available
	by the time we run out of partitioning.  Other protocols, like the session layer, will work
	from the lower bits of the byte up in an enumerated fashion with the high bit clear.
	
	Framing on this protocol is best accomplished by having all of the data bytes keep the high
	bits clear.  This implies 7 bit characters.  This will work well with partitioning ServerTalk
	and GameTalk into different physical protocol discriminators because the 4 bits used for framing
	are much cheaper than the 8 bits that would be used for an _unstuffed_ framing character.  
	Stuffing obviously blows this even higher.  Going into GameTalk, while transparent based on
	the protocol discriminator/GT framing bit, requires that the framing code remember for the
	length of the GT frame that it is recieving a GT frame. 
	
	This means that PUAsyncReadCompletion needs to know the mode that it is in and frame accordingly.
	In GT framing, PUARC will be watching the high bit on frame boundaries (found because of fixed
	size frames).  When GT framing is expected but not found, if the entire byte is the first char
	of ST framing, the next byte is checked and GT mode ends if a ST frame sequence is matched.
	The definition here then distills down to the transition to GT is done by the protocol discrim
	high bit, the transition back to ST by the sense of a ST framing sequence.
	
	If there are 4 characters/28 bits in a frame, 22 bits of controller data leaves 6 bits.  Use
	of 6-bit CRC would eat all these and we only need 5 for this frame size anyways.  The extra
	bit is almost essential for distinguishing control frames, which will be used for NAK.
	So 28-5-1 leaves 22 bits of a data field for control frames, which is probably wisely set up
	as a partioned area.  Since the first use is sending sequence on NAK, and 8 bits is plenty for
	that, we may as well define the 22 bits as 14:8, partitioning as needed.
	
	Partitioning can be done like IP addresses.  Using partitioning allows single bit discrimination
	on space-critical types, but is of course inefficient.  The partitioning can be undefined until
	it is needed; suffice it to say that the first three bits are used for partitioning, leaving
	32 choices in a single byte enumeration such used in the ST protocol discriminator.
	
Better approach.  Take advantage of the fact that a resend is going to leave a lot of dead airtime.
	If we have to do a resend, use the gap to back up and frame against that.  This frees up lots
	'o bits.  We can actually get back to doing 2 bytes for single player 
	(1 or 2 partitioning+11+csum).  2 player is still going to require 4 bytes, because (24-22) is not
	enough left over for a meaningful checksum.  This is best implemented by using the Syncotron
	timestamp to figure out when a character arrived.  None of this is implemented yet, assuming that
	all bytes arrive and framing is not a problem.
	
	There is one problem with having GT/ST frames coresident.  In the transition between them, it is
	undefined how to find framing again if it is lost during transition.  There is no forward reset
	at the physical layer (but this is probably the answer) to get them synched.  Keep the whole proto
	modal, keep the features that it might someday work, and leave it at that.

Different Approach.  Packet modes are officially modal.  Model is that I don't
	care what is in the data, the client picks how he wants to transfer and I just get the bits there.  There
	is constant overhead on each different packet size.  This reads as GT1 has 4 bits for data, GT2 has 11 bits,
	and GT3 has 19 bits, and GT4 probably has 26 bits (with a 5 bit checksum [?]).
	
	Error correction is by recording the sequence number on both sides of the connection and asking for 
	a retrans on the appropriate frame.  Since there is only a limited fixed size fifo, it needs to be larger
	than the longest latency we will ever encounter * 2, to cover for the round trip delay.  We also need
	to add some slop for the processing delay.  When a bad packet is detected, send that sequence number and
	wait for it to return.  The return stream is preceeded by a resync packet, which is a ctl packet with
	a resync code.  The resync packet is sent on a VBL, just like the other packets.  The time delay around
	a resync gives framing information.  Since the resync has a checksum itself, it may be detected as
	unrecieved if sent improperly or not at all.  Unrecieved is detected by no filtered control packet
	being recieved before a max latency watchdog times out.
	
	We also have to consider that syncotron is going have to be turned off when we realize that a bad packet
	arrived.  We don't want it blowing up the sync on the games.   
	
	This whole mess needs to be vectored.  This is to say that there  

Also need to get things ready to be a real server.  This includes putting in the code so that
the lower layers can be sitting idle while the higher layers are inactive.  This probably only
means moving the lower level init code into the init for the app.  Start by making the TListen
code deny any connection requests.

Need to add a RegisterPListener call to the phys layer so we can patch in new higher layer
protocols easily.

FifoWrite is killing the cache when it doesn't check for kCachingQ

Add code to Session to idle if there is no room to write the data instead of hoarking

Working on this concept of error propagation.  The current idea is that there will be a stateless
callback that puts up user dialog when there is going to be a network delay of any sort.  This
guy gets called as a subchain of NetIdle, so there is something of a guarantee that things are
safe for allocation and drawing.  This proc gets called with a message of what to display and
an ETA for when the process will be done.  The ETA can be used to display a part of the user
notification.  The dialog should be left around until the routine is called with an ETA of zero.
The callback is always guaranteed to call once with a value of zero, indicating that the wait
is over.  

This architecture is important as we get recoverable interruptions of service.  Returning from
a sync read without a full buffer will trash what has been moved across the line already and
neccesitate that it be resent.  Not returning from the read means that no user interface can be
presented.

If there is a fatal error that occurs with the dialog up, the proc will be called with an ETA 
of zero, the sync read will abort and return an appropriate error.  When a fatal error
propagates in this way, it needs to be defined whether the client or the session has the authority
to flush outstanding data and/or close the connection.  If it is up to the client to close the
connection, the data will be flushed then.  This is where the concept of a half-open connection
and the correct handling of it starts to become important.



*/

/*

Next thing to start on:  Packet trace window for protocol test app.  This would be something
of an integration of the serial tool, but would get in the layers of the existing prot