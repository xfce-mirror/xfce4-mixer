/*
 * Copyright (c) 2004-2005 Danny Milosavljevic <dannym@xfce.org>
 * Copyright (c) 2006 Daichi Kawahata <daichi@xfce.org>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include <libxfce4util/libxfce4util.h>

/*
 * OSS translateable names
 */

char *oss_names[] = {
	_("Vol"),			/* Master				*/
	_("Pcm"),			/* PCM output			*/
	_("Spkr"),			/* Internal speaker?	*/
	_("Line"),			/* Line in				*/
	_("Mic"),			/* Mic in				*/
	_("CD"),			/* CD in				*/
	_("Bass"),
	_("Trebl"),
	_("Synth"),
	_("Mix"),
	_("Pcm2"),
	_("Rec"),
	_("IGain"),
	_("OGain"),
	_("Line1"),
	_("Line2"),
	_("Line3"),
	_("Digital1"),
	_("Digital2"),
	_("Digital3"),
	_("PhoneIn"),
	_("PhoneOut"),
	_("Video"),
	_("Radio"),
	_("Monitor"),
	_("RecSelect"),		/* I made that up */
};

/*
 * IRIX AL translatable names
 *
 * The strings below come from SGI O2
 * (Iris Audio Processor: version A3 revision 0)
 *
 * If you have an another processor (e.g. A2, RAD, MAD) and
 * Device/Interface names, let me <daichi@xfce.org> know.
 * 
 */

char *sgi_names[] = {
	/* Device */			/* Label	 		*/
	_("Analog In"),			/* A3.AnalogIn		*/
	_("Analog Out"),		/* A3.AnalogOut		*/
	_("Analog Out 2"),		/* A3.AnalogOut2	*/
	/* Interface */
	_("Camera Mic"),		/* A3.CameraMic		*/
	_("Microphone"),		/* A3.Microphone	*/
	_("Line In"),			/* A3.LineIn		*/
	_("DAC1 In"),			/* A3.DAC1In		*/
	_("DAC2 In"),			/* A3.DAC2In		*/
	_("Speaker/Line Out"),	/* A3.Speaker		*/
	_("Line Out 2"),		/* A3.LineOut2		*/
};

/* vi: set ts=4 sw=4 cindent: */

