#import <Cocoa/Cocoa.h>

#include "GestureRecognizer.hpp"
#include "GestureWindow.hpp"

#include <exception>

namespace {
constexpr double idleSeconds = 0.3;

NSTextField* addLabel(NSView* view, NSString* text, NSRect frame, CGFloat fontSize) {
    NSTextField* label = [NSTextField wrappingLabelWithString:text];
    label.frame = frame;
    label.font = [NSFont systemFontOfSize:fontSize];
    [view addSubview:label];
    return label;
}
}

@interface GestureView : NSView <NSWindowDelegate> {
    const GestureRecognizer* _recognizer;
    NSString* _input;
    NSTextField* _inputLabel;
    NSTextField* _resultLabel;
    NSTimer* _timer;
    NSTimeInterval _lastInputTime;
}
- (instancetype)initWithRecognizer:(const GestureRecognizer&)recognizer;
- (void)recognize:(NSTimer*)timer;
@end

@implementation GestureView
- (instancetype)initWithRecognizer:(const GestureRecognizer&)recognizer {
    self = [super initWithFrame:NSMakeRect(0, 0, 660, 420)];
    if (self) {
        _recognizer = &recognizer;
        _input = @"";
        addLabel(self, @"Type in this window. Pause for 0.3 seconds to recognize a gesture.",
                 NSMakeRect(24, 360, 612, 36), 16);
        _inputLabel = addLabel(self, @"Input: waiting for keys…",
                               NSMakeRect(24, 278, 612, 66), 18);
        _resultLabel = addLabel(self, @"Results will appear here.",
                                NSMakeRect(24, 24, 612, 230), 18);
    }
    return self;
}

- (BOOL)acceptsFirstResponder { return YES; }

- (void)mouseDown:(NSEvent*)event {
    [self.window makeFirstResponder:self];
}

- (void)keyDown:(NSEvent*)event {
    NSString* characters = event.characters;
    if (characters.length == 0 ||
        (event.modifierFlags & (NSEventModifierFlagCommand | NSEventModifierFlagControl))) {
        return;
    }
    for (NSUInteger i = 0; i < characters.length; ++i) {
        const unichar c = [characters characterAtIndex:i];
        // Navigation and control keys are not part of a gesture sample.
        if (c < 0x20 || c == 0x7F || (c >= 0xF700 && c <= 0xF8FF)) {
            return;
        }
    }

    // Keep gestures separate even if the run loop delivered the timer late.
    if (_input.length > 0 && event.timestamp - _lastInputTime >= idleSeconds) {
        [self recognize:nil];
    }
    _lastInputTime = event.timestamp;
    _input = [_input stringByAppendingString:characters];
    _inputLabel.stringValue = [@"Input: " stringByAppendingString:_input];
    _resultLabel.stringValue = @"Listening…";

    [_timer invalidate];
    _timer = [NSTimer scheduledTimerWithTimeInterval:idleSeconds
                                            target:self
                                          selector:@selector(recognize:)
                                          userInfo:nil
                                           repeats:NO];
}

- (void)recognize:(NSTimer*)timer {
    [_timer invalidate];
    _timer = nil;
    if (_input.length == 0) {
        return;
    }

    try {
        const auto results = _recognizer->rank(_input.UTF8String);
        if (results.empty() || results.front().score == 0.0) {
            _resultLabel.stringValue = @"No matching gesture.";
        } else {
            NSMutableString* text = [NSMutableString stringWithFormat:@"Detected: %s\n\n",
                GestureRecognizer::gestureName(results.front().gesture).data()];
            for (std::size_t i = 0; i < results.size(); ++i) {
                [text appendFormat:@"%zu. %s   %.3f\n", i + 1,
                    GestureRecognizer::gestureName(results[i].gesture).data(), results[i].score];
            }
            _resultLabel.stringValue = text;
        }
    } catch (const std::exception& error) {
        _resultLabel.stringValue = [NSString stringWithUTF8String:error.what()];
    }
    _input = @"";
}

- (void)windowWillClose:(NSNotification*)notification {
    [_timer invalidate];
    [NSApp stop:nil];
}
@end

void runGestureWindow(const GestureRecognizer& recognizer) {
    @autoreleasepool {
        NSApplication* app = [NSApplication sharedApplication];
        [app setActivationPolicy:NSApplicationActivationPolicyRegular];

        NSMenu* menu = [NSMenu new];
        NSMenuItem* appMenu = [NSMenuItem new];
        [menu addItem:appMenu];
        appMenu.submenu = [NSMenu new];
        [appMenu.submenu addItemWithTitle:@"Quit Gesture Recognition"
                                  action:@selector(terminate:) keyEquivalent:@"q"];
        app.mainMenu = menu;

        GestureView* view = [[GestureView alloc] initWithRecognizer:recognizer];
        NSWindow* window = [[NSWindow alloc] initWithContentRect:view.frame
            styleMask:NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskMiniaturizable
            backing:NSBackingStoreBuffered defer:NO];
        window.title = @"Gesture Recognition";
        window.releasedWhenClosed = NO;
        window.contentView = view;
        window.delegate = view;
        [window center];
        [app finishLaunching];
        [window makeKeyAndOrderFront:nil];
        [window makeFirstResponder:view];
        [app activateIgnoringOtherApps:YES];
        [app run];
    }
}
