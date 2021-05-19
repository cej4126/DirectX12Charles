#include "Input.h"

std::optional<Input::RawDelta> Input::readRawDelta() noexcept
{
   if (m_rawDeltaBuffer.empty())
   {
      return std::nullopt;
   }
   const RawDelta d = m_rawDeltaBuffer.front();
   m_rawDeltaBuffer.pop();
   return d;
}

//std::optional<Input::MouseEvent> Input::Read() noexcept
//{
//   if (mouseBuffer.size() > 0u)
//   {
//      Input::MouseEvent e = mouseBuffer.front();
//      mouseBuffer.pop();
//      return e;
//   }
//   return {};
//}

//void Input::Flush() noexcept
//{
//   mouseBuffer = std::queue<MouseEvent>();
//   keyBuffer = std::queue<KeyEvent>();
//   charBuffer = std::queue<char>();
//}

void Input::onKeyPressed(unsigned char keycode) noexcept
{
   m_keyStates[keycode] = true;
   m_keyBuffer.push(Input::KeyEvent(Input::KeyEvent::KeyType::KeyPress, keycode));
   trimBuffer(m_keyBuffer);
}

void Input::onKeyReleased(unsigned char keycode) noexcept
{
   m_keyStates[keycode] = false;
   m_keyBuffer.push(Input::KeyEvent(Input::KeyEvent::KeyType::KeyRelease, keycode));
   trimBuffer(m_keyBuffer);
}

void Input::onChar(char character) noexcept
{
   m_charBuffer.push(character);
   trimBuffer(m_charBuffer);
}

std::optional<Input::KeyEvent> Input::ReadKey() noexcept
{
   if (m_keyBuffer.size() > 0u)
   {
      Input::KeyEvent e = m_keyBuffer.front();
      m_keyBuffer.pop();
      return e;
   }
   return {};
}

//std::optional<char> Input::ReadChar() noexcept
//{
//   if (charBuffer.size() > 0u)
//   {
//      unsigned char charcode = charBuffer.front();
//      charBuffer.pop();
//      return charcode;
//   }
//   return {};
//}

void Input::onMouseMove(int newx, int newy) noexcept
{
   m_x = newx;
   m_y = newy;

   m_mouseBuffer.push(Input::MouseEvent(Input::MouseEvent::MouseType::Move, *this));
   trimBuffer();
}

void Input::onMouseLeave() noexcept
{
   m_isInWindow = false;
   m_mouseBuffer.push(Input::MouseEvent(Input::MouseEvent::MouseType::Leave, *this));
   trimBuffer();
}

void Input::onMouseEnter() noexcept
{
   m_isInWindow = true;
   m_mouseBuffer.push(Input::MouseEvent(Input::MouseEvent::MouseType::Enter, *this));
   trimBuffer();
}

void Input::onRawDelta(int dx, int dy) noexcept
{
   m_rawDeltaBuffer.push({ dx,dy });
   trimBuffer();
}

void Input::onLeftPressed(int x, int y) noexcept
{
   m_leftIsPressed = true;

   m_mouseBuffer.push(Input::MouseEvent(Input::MouseEvent::MouseType::LPress, *this));
   trimBuffer();
}

void Input::onLeftReleased(int x, int y) noexcept
{
   m_leftIsPressed = false;

   m_mouseBuffer.push(Input::MouseEvent(Input::MouseEvent::MouseType::LRelease, *this));
   trimBuffer();
}

void Input::onRightPressed(int x, int y) noexcept
{
   m_rightIsPressed = true;

   m_mouseBuffer.push(Input::MouseEvent(Input::MouseEvent::MouseType::RPress, *this));
   trimBuffer();
}

void Input::onRightReleased(int x, int y) noexcept
{
   m_rightIsPressed = false;

   m_mouseBuffer.push(Input::MouseEvent(Input::MouseEvent::MouseType::RRelease, *this));
   trimBuffer();
}

void Input::onWheelUp(int x, int y) noexcept
{
   m_mouseBuffer.push(Input::MouseEvent(Input::MouseEvent::MouseType::WheelUp, *this));
   trimBuffer();
}

void Input::onWheelDown(int x, int y) noexcept
{
   m_mouseBuffer.push(Input::MouseEvent(Input::MouseEvent::MouseType::WheelDown, *this));
   trimBuffer();
}

void Input::trimBuffer() noexcept
{
   while (m_mouseBuffer.size() > Buffer_Size)
   {
      m_mouseBuffer.pop();
   }
}

void Input::trimRawInputBuffer() noexcept
{
   while (m_rawDeltaBuffer.size() > Buffer_Size)
   {
      m_rawDeltaBuffer.pop();
   }
}

void Input::onWheelDelta(int x, int y, int delta) noexcept
{
   m_wheelDeltaCarry += delta;
   // generate events for every 120 
   while (m_wheelDeltaCarry >= WHEEL_DELTA)
   {
      m_wheelDeltaCarry -= WHEEL_DELTA;
      onWheelUp(x, y);
   }
   while (m_wheelDeltaCarry <= -WHEEL_DELTA)
   {
      m_wheelDeltaCarry += WHEEL_DELTA;
      onWheelDown(x, y);
   }
}


template<typename T>
inline void Input::trimBuffer(std::queue<T> &buffer) noexcept
{
   while (buffer.size() > Buffer_Size)
   {
      buffer.pop();
   }
}
