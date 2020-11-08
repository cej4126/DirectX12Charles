#pragma once
#include "stdafx.h"
#include <queue>
#include <bitset>
#include <optional>

class Input
{
   friend class Window;

public:
   struct RawDelta
   {
      int x;
      int y;
   };

   class MouseEvent
   {
   public:
      enum class MouseType
      {
         LPress,
         LRelease,
         RPress,
         RRelease,
         WheelUp,
         WheelDown,
         Move,
         Enter,
         Leave
      };
   public:
      MouseEvent(MouseType type, const Input &parent) noexcept
         :
         type(type),
         leftIsPressed(parent.leftIsPressed),
         rightIsPressed(parent.rightIsPressed),
         x(parent.x),
         y(parent.y)
      {}

      MouseType GetType() const noexcept { return type; }
      std::pair <int, int> GetPos() const noexcept { return { x, y }; }
      int GetPosX() { return x; }
      int GetPosY() { return y; }
      bool LeftIsPressed() const noexcept { return leftIsPressed; }
      bool RightIsPressed() const noexcept { return rightIsPressed; }

   private:
      MouseType type;
      bool leftIsPressed;
      bool rightIsPressed;
      int x;
      int y;
   };

   class KeyEvent
   {
   public:
      enum class KeyType
      {
         KeyPress,
         KeyRelease,
      };
   public:
      KeyEvent(KeyType type, unsigned char code) noexcept
         :
         type(type),
         code(code)
      {}

      KeyType GetType() const noexcept { return type; }
      int isPress() { return type == KeyType::KeyPress; }
      int isRelease() { return type == KeyType::KeyRelease; }
      int GetCode() { return code; }

   private:
      KeyType type;
      unsigned char code;
   };

public:
   Input() = default;
   Input(const Input &) = delete;
   Input &operator=(const Input &) = delete;

   // Mouse Functions
   std::pair <int, int> GetPos() const noexcept { return { x, y }; }
   std::optional<RawDelta> ReadRawDelta() noexcept;
   int GetPosX() { return x; }
   int GetPosY() { return y; }
   bool IsInWindow() { return isInWindow; }
   bool LeftIsPressed() const noexcept { return leftIsPressed; }
   bool RightIsPressed() const noexcept { return rightIsPressed; }
   std::optional<Input::MouseEvent> Read() noexcept;
   bool IsEmpty() const noexcept { return mouseBuffer.empty(); }
   void EnableRaw() noexcept { rawEnabled = true; }
   void DisableRaw() noexcept { rawEnabled = false; }
   bool RawEnabled() const noexcept { return rawEnabled; }

   // Key functions
   bool KeyIsPressed(unsigned char keycode) const noexcept { return keyStates[keycode]; }
   std::optional<KeyEvent> ReadKey() noexcept;
   bool KeyIsEmpty() const noexcept { return charBuffer.empty(); }
   std::optional<char> ReadChar() noexcept;
   bool CharIsEmpty() const noexcept { return charBuffer.empty(); }
   // autorepeat control
   void EnableAutorepeat() noexcept { autorepeatEnabled = true; }
   void DisableAutorepeat() noexcept { autorepeatEnabled = false; }
   bool AutorepeatIsEnabled() const noexcept { return autorepeatEnabled; }

   void Flush() noexcept;
private:
   // Key
   void OnKeyPressed(unsigned char keycode) noexcept;
   void OnKeyReleased(unsigned char keycode) noexcept;
   void OnChar(char character) noexcept;
   void ClearState() noexcept { keyStates.reset(); }
   template<typename T>
   static void TrimBuffer(std::queue<T> &buffer) noexcept;

   // Mouse
   void OnMouseMove(int x, int y) noexcept;
   void OnMouseLeave() noexcept;
   void OnMouseEnter() noexcept;
   void OnRawDelta(int dx, int dy) noexcept;
   void OnLeftPressed(int x, int y) noexcept;
   void OnLeftReleased(int x, int y) noexcept;
   void OnRightPressed(int x, int y) noexcept;
   void OnRightReleased(int x, int y) noexcept;
   void OnWheelUp(int x, int y) noexcept;
   void OnWheelDown(int x, int y) noexcept;
   void TrimBuffer() noexcept;
   void TrimRawInputBuffer() noexcept;
   void OnWheelDelta(int x, int y, int delta) noexcept;


private:
   static constexpr unsigned int bufferSize = 16u;
   int x;
   int y;
   bool leftIsPressed = false;
   bool rightIsPressed = false;
   bool isInWindow = false;
   int wheelDeltaCarry = 0;
   std::queue <MouseEvent> mouseBuffer;

   static constexpr unsigned int nKeys = 256u;
   bool autorepeatEnabled = false;
   bool rawEnabled = false;
   std::bitset<nKeys> keyStates;
   std::queue<KeyEvent> keyBuffer;
   std::queue<char> charBuffer;
   std::queue<RawDelta> rawDeltaBuffer;
};
