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
         m_type(type),
         m_leftIsPressed(parent.m_leftIsPressed),
         m_rightIsPressed(parent.m_rightIsPressed),
         m_x(parent.m_x),
         m_y(parent.m_y)
      {}

      //MouseType GetType() const noexcept { return type; }
      //std::pair <int, int> GetPos() const noexcept { return { x, y }; }
      //int GetPosX() { return x; }
      //int GetPosY() { return y; }
      //bool isLeftPressed() const noexcept { return leftIsPressed; }
      //bool isRightPressed() const noexcept { return rightIsPressed; }

   private:
      MouseType m_type;
      bool m_leftIsPressed;
      bool m_rightIsPressed;
      int m_x;
      int m_y;
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
         m_type(type),
         m_code(code)
      {}

      KeyType GetType() const noexcept { return m_type; }
      int isPress() { return m_type == KeyType::KeyPress; }
      int isRelease() { return m_type == KeyType::KeyRelease; }
      int GetCode() { return m_code; }

   private:
      KeyType m_type;
      unsigned char m_code;
   };

public:
   Input() = default;
   Input(const Input &) = delete;
   Input &operator=(const Input &) = delete;

   // Mouse Functions
   std::pair <int, int> getPos() const noexcept { return { m_x, m_y }; }
   std::optional<RawDelta> readRawDelta() noexcept;
   int getPosX() { return m_x; }
   int getPosY() { return m_y; }
   bool isInWindow() { return m_isInWindow; }
   bool leftIsPressed() const noexcept { return m_leftIsPressed; }
   bool rightIsPressed() const noexcept { return m_rightIsPressed; }
   //std::optional<Input::MouseEvent> Read() noexcept;
   bool isEmpty() const noexcept { return m_mouseBuffer.empty(); }
   void enableRaw() noexcept { m_rawEnabled = true; }
   void disableRaw() noexcept { m_rawEnabled = false; }
   bool rawEnabled() const noexcept { return m_rawEnabled; }

   // Key functions
   bool keyIsPressed(unsigned char keycode) const noexcept { return m_keyStates[keycode]; }
   std::optional<KeyEvent> ReadKey() noexcept;
   bool keyIsEmpty() const noexcept { return m_charBuffer.empty(); }
   //std::optional<char> ReadChar() noexcept;
   bool charIsEmpty() const noexcept { return m_charBuffer.empty(); }
   // autorepeat control
   void enableAutorepeat() noexcept { m_autorepeatEnabled = true; }
   void disableAutorepeat() noexcept { m_autorepeatEnabled = false; }
   bool autorepeatIsEnabled() const noexcept { return m_autorepeatEnabled; }

   //void Flush() noexcept;
private:
   // Key
   void onKeyPressed(unsigned char keycode) noexcept;
   void onKeyReleased(unsigned char keycode) noexcept;
   void onChar(char character) noexcept;
   void clearState() noexcept { m_keyStates.reset(); }
   template<typename T>
   static void trimBuffer(std::queue<T> &buffer) noexcept;

   // Mouse
   void onMouseMove(int x, int y) noexcept;
   void onMouseLeave() noexcept;
   void onMouseEnter() noexcept;
   void onRawDelta(int dx, int dy) noexcept;
   void onLeftPressed(int x, int y) noexcept;
   void onLeftReleased(int x, int y) noexcept;
   void onRightPressed(int x, int y) noexcept;
   void onRightReleased(int x, int y) noexcept;
   void onWheelUp(int x, int y) noexcept;
   void onWheelDown(int x, int y) noexcept;
   void trimBuffer() noexcept;
   void trimRawInputBuffer() noexcept;
   void onWheelDelta(int x, int y, int delta) noexcept;


private:
   static constexpr unsigned int Buffer_Size = 16u;
   int m_x;
   int m_y;
   bool m_leftIsPressed = false;
   bool m_rightIsPressed = false;
   bool m_isInWindow = false;
   int m_wheelDeltaCarry = 0;
   std::queue <MouseEvent> m_mouseBuffer;

   static constexpr unsigned int N_Keys = 256u;
   bool m_autorepeatEnabled = false;
   bool m_rawEnabled = false;
   std::bitset<N_Keys> m_keyStates;
   std::queue<KeyEvent> m_keyBuffer;
   std::queue<char> m_charBuffer;
   std::queue<RawDelta> m_rawDeltaBuffer;
};
