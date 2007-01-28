/*
	File:		HIScrollingTextBox.cp

	Contains:	Demonstrates creating a simple embedder using the HIView APIs.

	Version:	1.0

	Disclaimer:	IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
				("Apple") in consideration of your agreement to the following terms, and your
				use, installation, modification or redistribution of this Apple software
				constitutes acceptance of these terms.  If you do not agree with these terms,
				please do not use, install, modify or redistribute this Apple software.

				In consideration of your agreement to abide by the following terms, and subject
				to these terms, Apple grants you a personal, non-exclusive license, under Apple�s
				copyrights in this original Apple software (the "Apple Software"), to use,
				reproduce, modify and redistribute the Apple Software, with or without
				modifications, in source and/or binary forms; provided that if you redistribute
				the Apple Software in its entirety and without modifications, you must retain
				this notice and the following text and disclaimers in all such redistributions of
				the Apple Software.  Neither the name, trademarks, service marks or logos of
				Apple Computer, Inc. may be used to endorse or promote products derived from the
				Apple Software without specific prior written permission from Apple.  Except as
				expressly stated in this notice, no other rights or licenses, express or implied,
				are granted by Apple herein, including but not limited to any patent rights that
				may be infringed by your derivative works or by other works in which the Apple
				Software may be incorporated.

				The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO
				WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED
				WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR
				PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE OR IN
				COMBINATION WITH YOUR PRODUCTS.

				IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR
				CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
				GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
				ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION
				OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER UNDER THEORY OF CONTRACT, TORT
				(INCLUDING NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN
				ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

	Copyright � 2003 Apple Computer, Inc., All Rights Reserved
*/

#include <Carbon/Carbon.h>

const ControlID kScrollingTextBoxViewID = {'CASD','HSTB'};
#define kScrollingTextBoxClassID CFSTR("com.apple.sample.dts.HIScrollingTextBox2")

#define kSetText 'stxt'

#define HI2QDRECT(hir) { (int)hir.origin.y, (int)hir.origin.x, (int)(hir.origin.y + hir.size.height), (int)(hir.origin.x + hir.size.width) }

#include "Utils.c"

#define kMargin 3.0
#define kScrollingTextBoxFontID kThemeSystemFont

#define PANTHER_BUILD 0
#define Panther_HIToolbox_Version 0x130

// structure in which we hold our custom push button's data
typedef struct
	{
	HIViewRef	      view;							// the HIViewRef for our list
	CFStringRef	      theText;
	Boolean		      autoScroll;
	UInt32		      delayBeforeAutoScroll;
	UInt32		      delayBetweenAutoScroll;
	UInt16		      autoScrollAmount;
	HIPoint		      originPoint;
	float			      height;
	EventLoopTimerRef theTimer;
	}
ScrollingTextBoxData;

OSStatus ScrollingTextBoxDraw(CGContextRef context, const HIRect * bounds, const ScrollingTextBoxData * myData)
	{
	HIRect textBounds = { {kMargin, kMargin}, {bounds->size.width - kMargin - kMargin, myData->height} };
	HIRect clipBounds = CGRectInset(*bounds, kMargin + 1.0, kMargin + 1.0);

//
// If we're building on Panther (or later) then HIThemeDrawTextBox is available, else we use DrawThemeTextBox
//
#if PANTHER_BUILD
//
// Furthermore, if we're running on Panther then we can call HIThemeDrawTextBox else we call DrawThemeTextBox
//
	if (GetHIToolboxVersion() >= Panther_HIToolbox_Version)
		{
		CGContextClipToRect(context, clipBounds);

		HIThemeTextInfo textInfo = {0, kThemeStateActive, kScrollingTextBoxFontID, kHIThemeTextHorizontalFlushLeft, kHIThemeTextVerticalFlushTop, kHIThemeTextBoxOptionStronglyVertical, kHIThemeTextTruncationNone, 0, false};
		HIThemeDrawTextBox(myData->theText, &textBounds, &textInfo, context, kHIThemeOrientationNormal);
		}
	else
#endif
		{
		Rect QDTextBounds = HI2QDRECT(textBounds);
		Rect QDClipBounds = HI2QDRECT(clipBounds);

		RgnHandle saveClip = NewRgn();
		GetClip(saveClip);
		ClipRect(&QDClipBounds);

		DrawThemeTextBox(myData->theText, kScrollingTextBoxFontID, kThemeStateActive, true, &QDTextBounds, teJustLeft, context);

		SetClip(saveClip);
		DisposeRgn(saveClip);
		}

	return noErr;
	}

pascal OSStatus ScrollingTextViewHandler(EventHandlerCallRef inCaller, EventRef inEvent, void* inRefcon);
CFStringRef GetScrollingTextBoxClass();

/*
 *  HICreateScrollingTextBox()
 *
 *  Summary:
 *    Creates a scrolling mono-style text box HIView based on the scrolling text box control (CDEF 27).
 *    Differences:
 *    -------------
 *    - It takes a CFStringRef instead of a 'TEXT'/'styl' resource ID pair so we get mon-style Unicode text.
 *    - It doesn't need a WindowRef parameter.
 *
 *  Parameters:
 *
 *    inBounds:
 *      The initial bounds of the view. If this parameter is NULL, the
 *      view defaults to have empty bounds ( 0, 0, 0, 0 ).
 *
 *    inScrollingText:
 *      The text to be displayed in the box.
 *
 *    inAutoScroll:
 *      If true then the text scrolls automatically using the following parameters,
 *      If false then the text has to be scrolled manually using the visible scroll bar.
 *
 *    inDelayBeforeAutoScroll:
 *      If inAutoScroll is true, then this is the delay before the automatic scrolling starts,
 *      If inAutoScroll is false, this parameter is ignored.
 *
 *    inDelayBetweenAutoScroll:
 *      If inAutoScroll is true, then this parameter and the next one set the automatic scrolling speed,
 *      If inAutoScroll is false, this parameter is ignored.
 *
 *    inAutoScrollAmount:
 *      If inAutoScroll is true, then this parameter and the previous one set the automatic scrolling speed,
 *      If inAutoScroll is false, this parameter is ignored.
 *
 *    outHIView:
 *      On exit, contains the new HIView.
 *
 *  Availability:
 *    Mac OS X:         in version 10.2 and later since it needs the HIView APIs
 *    CarbonLib:        not available
 *    Non-Carbon CFM:   not available
 */

enum {
  kEventParamScrollingText           = 'text', /*    typeCFStringRef*/
  kEventParamAutoScroll              = 'auto', /*    typeBoolean*/
  kEventParamDelayBeforeAutoScroll   = 'dbfa', /*    typeUInt32*/
  kEventParamDelayBetweenAutoScroll  = 'dbta', /*    typeUInt32*/
  kEventParamAutoScrollAmount        = 'asam'  /*    typeSInt16*/
};

extern OSStatus
HICreateScrollingTextBox(
	const HIRect * inBounds,                   /* can be NULL */
	CFStringRef    inScrollingText,
	Boolean        inAutoScroll,
	UInt32         inDelayBeforeAutoScroll,
	UInt32         inDelayBetweenAutoScroll,
	UInt16         inAutoScrollAmount,
	HIViewRef *    outHIView)
	{
	*outHIView = NULL;
	EventRef theInitializeEvent = NULL;
	HIViewRef scrollView;
	OSStatus status;

	status = CreateEvent(NULL, kEventClassHIObject, kEventHIObjectInitialize, GetCurrentEventTime(), kEventAttributeUserEvent, &theInitializeEvent);

	// settings
	SetEventParameter(theInitializeEvent, kEventParamScrollingText, typeCFStringRef, sizeof(inScrollingText), &inScrollingText);
	SetEventParameter(theInitializeEvent, kEventParamAutoScroll, typeBoolean, sizeof(inAutoScroll), &inAutoScroll);
	SetEventParameter(theInitializeEvent, kEventParamDelayBeforeAutoScroll, typeUInt32, sizeof(inDelayBeforeAutoScroll), &inDelayBeforeAutoScroll);
	SetEventParameter(theInitializeEvent, kEventParamDelayBetweenAutoScroll, typeUInt32, sizeof(inDelayBetweenAutoScroll), &inDelayBetweenAutoScroll);
	SetEventParameter(theInitializeEvent, kEventParamAutoScrollAmount, typeSInt16, sizeof(inAutoScrollAmount), &inAutoScrollAmount);

	HIObjectRef hiObject;
	status = HIObjectCreate(GetScrollingTextBoxClass(), theInitializeEvent, &hiObject);

	HIViewSetVisible((HIViewRef)hiObject, true);

	if (!inAutoScroll)
		{
		//
		// Manual scrolling, we need to be embedded in a scroll view
		//
		status = HIScrollViewCreate(kHIScrollViewOptionsVertScroll, &scrollView);
		status = HIViewAddSubview(scrollView, (HIViewRef)hiObject);
		if (inBounds != NULL)
			HIViewSetFrame(scrollView, inBounds);
		EventTypeSpec event = {kEventClassControl, kEventControlDraw};
		InstallEventHandler(GetControlEventTarget(scrollView), FrameView, 1, &event, NULL, NULL);
		*outHIView = scrollView;
		}
	else
		{
		if (inBounds != NULL)
			HIViewSetFrame((HIViewRef)hiObject, inBounds);
		*outHIView = (HIViewRef)hiObject;
		}

	return status;
	}

/*----------------------------------------------------------------------------------------------------------*/
//	� GetScrollingTextBoxClass
//	Registers and returns an HIObject class for our control.
/*----------------------------------------------------------------------------------------------------------*/
CFStringRef GetScrollingTextBoxClass()
	{

	// following code is pretty much boiler plate.

	static HIObjectClassRef	theClass;

	if (theClass == NULL)
		{
		static EventTypeSpec kFactoryEvents[] =
			{
				// the next 3 messages are boiler plate

				{ kEventClassHIObject, kEventHIObjectConstruct },
				{ kEventClassHIObject, kEventHIObjectInitialize },
				{ kEventClassHIObject, kEventHIObjectDestruct },

				// the next 2 messages are Scroll specific

				{ kEventClassScrollable, kEventScrollableGetInfo },
				{ kEventClassScrollable, kEventScrollableScrollTo },

				// the next message is Control specific

				{ kEventClassControl, kEventControlInitialize },
				{ kEventClassControl, kEventControlBoundsChanged },
				{ kEventClassControl, kEventControlDraw },
				{ kEventClassControl, kEventControlSetData },
			};

		HIObjectRegisterSubclass(kScrollingTextBoxClassID, kHIViewClassID, 0, ScrollingTextViewHandler,
								  GetEventTypeCount(kFactoryEvents), kFactoryEvents, 0, &theClass);
		}

	return kScrollingTextBoxClassID;
	}

/*----------------------------------------------------------------------------------------------------------*/
//	� myScrollingTextTimeProc
//	Carbon Event Timer Proc for our autoscroll.
/*----------------------------------------------------------------------------------------------------------*/
void myScrollingTextTimeProc(EventLoopTimerRef inTimer, void *inUserData)
	{
	ScrollingTextBoxData* myData = (ScrollingTextBoxData*)inUserData;
	HIPoint where = {0.0, myData->originPoint.y + myData->autoScrollAmount};

	// If we reached the end of our text, let's start again
	if (where.y >= myData->height - 10.0) where.y = 0.0;

	// Creating and sending our ScrollTo event
	EventRef theEvent;
	CreateEvent(NULL, kEventClassScrollable, kEventScrollableScrollTo, GetCurrentEventTime(), kEventAttributeUserEvent, &theEvent);
	SetEventParameter(theEvent, kEventParamOrigin, typeHIPoint, sizeof(where), &where);
	SendEventToEventTarget(theEvent, GetControlEventTarget(myData->view));
	ReleaseEvent(theEvent);
	}

/*----------------------------------------------------------------------------------------------------------*/
//	� ScrollingTextViewHandler
//	Event handler that implements our control.
/*----------------------------------------------------------------------------------------------------------*/
pascal OSStatus ScrollingTextViewHandler(EventHandlerCallRef inCaller, EventRef inEvent, void* inRefcon)
	{
	OSStatus result = eventNotHandledErr;
	ScrollingTextBoxData* myData = (ScrollingTextBoxData*)inRefcon;

	switch (GetEventClass(inEvent))
		{

		case kEventClassHIObject:
			switch (GetEventKind(inEvent))
				{
				case kEventHIObjectConstruct:
					{
					// allocate some instance data
					myData = (ScrollingTextBoxData*) calloc(1, sizeof(ScrollingTextBoxData));

					// get our superclass instance
					HIViewRef epView;
					GetEventParameter(inEvent, kEventParamHIObjectInstance, typeHIObjectRef, NULL, sizeof(epView), NULL, &epView);

					// remember our superclass in our instance data and initialize other fields
					myData->view = epView;

					// set the control ID so that we can find it later with HIViewFindByID
					result = SetControlID(myData->view, &kScrollingTextBoxViewID);

					// store our instance data into the event
					result = SetEventParameter(inEvent, kEventParamHIObjectInstance, typeVoidPtr, sizeof(myData), &myData);

					break;
					}

				case kEventHIObjectDestruct:
					{
					if (myData->theTimer != NULL) RemoveEventLoopTimer(myData->theTimer);
					CFRelease(myData->theText);
					free(myData);
					result = noErr;
					break;
					}

				case kEventHIObjectInitialize:
					{
					// always begin kEventHIObjectInitialize by calling through to the previous handler
					result = CallNextEventHandler(inCaller, inEvent);

					// if that succeeded, do our own initialization
					if (result == noErr)
						{
						GetEventParameter(inEvent, kEventParamScrollingText, typeCFStringRef, NULL, sizeof(myData->theText), NULL, &myData->theText);
						CFRetain(myData->theText);
						GetEventParameter(inEvent, kEventParamAutoScroll, typeBoolean, NULL, sizeof(myData->autoScroll), NULL, &myData->autoScroll);
						GetEventParameter(inEvent, kEventParamDelayBeforeAutoScroll, typeUInt32, NULL, sizeof(myData->delayBeforeAutoScroll), NULL, &myData->delayBeforeAutoScroll);
						GetEventParameter(inEvent, kEventParamDelayBetweenAutoScroll, typeUInt32, NULL, sizeof(myData->delayBetweenAutoScroll), NULL, &myData->delayBetweenAutoScroll);
						GetEventParameter(inEvent, kEventParamAutoScrollAmount, typeSInt16, NULL, sizeof(myData->autoScrollAmount), NULL, &myData->autoScrollAmount);
						myData->theTimer = NULL;
						}
					break;
					}

				default:
					break;
				}
			break;

		case kEventClassScrollable:
			switch (GetEventKind(inEvent))
				{
				case kEventScrollableGetInfo:
					{
					// we're being asked to return information about the scrolled view that we set as Event Parameters
					HISize imageSize = {50.0, myData->height};
					SetEventParameter(inEvent, kEventParamImageSize, typeHISize, sizeof(imageSize), &imageSize);
					HISize lineSize = {50.0, 20.0};
					SetEventParameter(inEvent, kEventParamLineSize, typeHISize, sizeof(lineSize), &lineSize);

					HIRect bounds;
					HIViewGetBounds(myData->view, &bounds);
					SetEventParameter(inEvent, kEventParamViewSize, typeHISize, sizeof(bounds.size), &bounds.size);
					SetEventParameter(inEvent, kEventParamOrigin, typeHIPoint, sizeof(myData->originPoint), &myData->originPoint);
					result = noErr;
					break;
					}

				case kEventScrollableScrollTo:
					{
					// we're being asked to scroll, we just do a sanity check and ask for a redraw
					HIPoint where;
					GetEventParameter(inEvent, kEventParamOrigin, typeHIPoint, NULL, sizeof(where), NULL, &where);

					HIViewSetNeedsDisplay(myData->view, true);

					myData->originPoint.y = (where.y < 0.0)?0.0:where.y;
					HIViewSetBoundsOrigin(myData->view, 0, myData->originPoint.y);

					break;
					}

				default:
					break;
				}
			break;

		case kEventClassControl:
			switch (GetEventKind(inEvent))
				{

				//	sets the feature of the view.

				case kEventControlInitialize:
					{
					result = CallNextEventHandler(inCaller, inEvent);
					if (result != noErr) break;

					UInt32 features = 0;
					result = GetEventParameter(inEvent, kEventParamControlFeatures, typeUInt32, NULL, sizeof(features), NULL, &features);
					if (result == noErr)
						features |= kControlSupportsEmbedding;
					else
						features = kControlSupportsEmbedding;

					result = SetEventParameter(inEvent, kEventParamControlFeatures, typeUInt32, sizeof features, &features);

					break;
					}

				//	Our parent view just changed dimensions, so we determined our new height.

				case kEventControlSetData:
					CFRelease(myData->theText);
					CFStringRef *p;
					GetEventParameter(inEvent, kEventParamControlDataBuffer, typePtr, NULL, sizeof(p), NULL, &p);
					myData->theText = *p;
					CFRetain(myData->theText);
					// fallthrough

				case kEventControlBoundsChanged:
					{
					HIRect bounds;
					HIViewGetBounds(myData->view, &bounds);

//
// If we're building on Panther (or later) then HIThemeGetTextDimensions is available, else we use GetThemeTextDimensions
//
#if PANTHER_BUILD
//
// Furthermore, if we're running on Panther then we can call HIThemeGetTextDimensions else we call GetThemeTextDimensions
//
					if (GetHIToolboxVersion() >= Panther_HIToolbox_Version)
						{
						HIThemeTextInfo textInfo = {0, kThemeStateActive, kScrollingTextBoxFontID, kHIThemeTextHorizontalFlushLeft, kHIThemeTextVerticalFlushTop, kHIThemeTextBoxOptionStronglyVertical, kHIThemeTextTruncationNone, 0, false};
						HIThemeGetTextDimensions(myData->theText, bounds.size.width - kMargin - kMargin, &textInfo, NULL, &myData->height, NULL);
						}
					else
#endif
						{
						Point pointBounds;
						pointBounds.h = (int)(bounds.size.width - kMargin - kMargin);
						GetThemeTextDimensions(myData->theText, kScrollingTextBoxFontID, kThemeStateActive, true, &pointBounds, NULL);
						myData->height = pointBounds.v;
						}

					myData->height += 2.0 * kMargin;

					HIViewSetNeedsDisplay(myData->view, true);

					result = eventNotHandledErr;
					break;
					}

				//	Draw the view.

				case kEventControlDraw:
					{
					CGContextRef context;
					result = GetEventParameter(inEvent, kEventParamCGContextRef, typeCGContextRef, NULL, sizeof(context), NULL, &context);

					HIRect bounds;
					HIViewGetBounds(myData->view, &bounds);

					CGContextSaveGState(context);
					CGAffineTransform transform = CGAffineTransformIdentity;
					// adjust the transform so the text doesn't draw upside down
					transform = CGAffineTransformScale(transform, 1, -1);
					CGContextSetTextMatrix(context, transform);

					// now that the proper parameters and configurations have been dealt with, let's draw
					result = ScrollingTextBoxDraw(context, &bounds, myData);

					CGContextRestoreGState(context);

					if (myData->autoScroll)
						CGContextStrokeRect(context, bounds);

					// we postpone starting the autoscroll timer until after we do our first drawing
					if ( (myData->autoScroll) && (myData->theTimer == NULL) )
						InstallEventLoopTimer(GetCurrentEventLoop(), TicksToEventTime(myData->delayBeforeAutoScroll), TicksToEventTime(myData->delayBetweenAutoScroll), myScrollingTextTimeProc, myData, &myData->theTimer);

					result = noErr;
					break;
					}

				default:
					break;
				}
			break;

		default:
			break;
		}

	return result;
	}
