#include "ScintillaBase.h"

#ifndef SCINTILLA_AWTK_H
#define SCINTILLA_AWTK_H

namespace Scintilla {

class ScintillaAWTK : public ScintillaBase {
 public:
  ScintillaAWTK(WindowID wid);
  virtual ~ScintillaAWTK();

  void OnPaint(widget_t* widget, canvas_t* c);
  void Invalidate(void);

  virtual void CreateCallTipWindow(PRectangle rc) override;
  virtual void AddToPopUp(const char* label, int cmd = 0, bool enabled = true) override;
  virtual void Initialise() override;
  virtual void SetVerticalScrollPos() override;
  virtual void SetHorizontalScrollPos() override;
  virtual bool ModifyScrollBars(Sci::Line nMax, Sci::Line nPage) override;
  virtual void Copy() override;
  virtual void Paste() override;
  virtual bool CanPaste() override;
  virtual void ClaimSelection() override;
  virtual void NotifyChange() override;
  virtual void NotifyParent(SCNotification scn) override;
  virtual void CopyToClipboard(const SelectionText& selectedText) override;
  virtual void SetMouseCapture(bool on) override;
  virtual bool HaveMouseCapture() override;
  virtual sptr_t DefWndProc(unsigned int iMessage, uptr_t wParam, sptr_t lParam) override;

  virtual PRectangle GetClientRectangle() const override;
  virtual bool FineTickerRunning(TickReason reason) override;
  virtual void FineTickerStart(TickReason reason, int millis, int tolerance) override;
  virtual void FineTickerCancel(TickReason reason) override;
  virtual bool SetIdle(bool) override;

 public:
  void AddText(const char* str);
  void ScrollAdd(int32_t delta, bool moveThumb);
  void OnScrollBarChange(const char* name, int32_t value);
  void SetClient(int32_t x, int32_t y, int32_t w, int32_t h);
  ret_t OnPointerDown(pointer_event_t* e);
  ret_t OnPointerMove(pointer_event_t* e);
  ret_t OnPointerUp(pointer_event_t* e);
  ret_t OnKeyDown(key_event_t* e);
  ret_t OnKeyUp(key_event_t* e);
  ret_t InsertString(const char* str);

 private:
  struct TimeThunk {
    uint32_t timer;
    TickReason reason;
    ScintillaAWTK* scintilla;
    TimeThunk() noexcept : timer(0), reason(tickCaret), scintilla(nullptr) {
    }
  };

  Point last_point;
  PRectangle client;
  uint32_t idle_id;
  TimeThunk timers[tickDwell + 1];
  bool lastKeyDownConsumed;
  widget_t* widget;
  bool_t bar_to_edit;
  static ret_t OnIdle(const idle_info_t* info);
  static ret_t OnTimeout(const timer_info_t* info);
};
};  // namespace Scintilla

#define VBAR_NAME "vbar"
#define HBAR_NAME "hbar"

#define SCI_SELECTNONE 0x10000
#define SCI_CANCOPY 0x10001
#define SCI_CANCUT 0x10002

#endif /*SCINTILLA_AWTK_H*/
