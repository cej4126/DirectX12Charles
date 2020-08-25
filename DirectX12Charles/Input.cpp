#include "Input.h"

std::optional<Input::MouseEvent> Input::Read() noexcept
{
   if (mouseBuffer.size() > 0u)
   {
      Input::MouseEvent e = mouseBuffer.front();
      mouseBuffer.pop();
      return e;
   }
   return {};
}

void Input::Flush() noexcept
{
   mouseBuffer = std::queue<MouseEvent>();
   keyBuffer = std::queue<KeyEvent>();
   charBuffer = std::queue<char>();
}

void Input::OnKeyPressed(unsigned char keycode) noexcept
{
   keyStates[keycode] = true;
   keyBuffer.push(Input::KeyEvent(Input::KeyEvent::KeyType::KeyPress, keycode));
   TrimBuffer(keyBuffer);
}

void Input::OnKeyReleased(unsigned char keycode) noexcept
{
   keyStates[keycode] = false;
   keyBuffer.push(Input::KeyEvent(Input::KeyEvent::KeyType::KeyRelease, keycode));
   TrimBuffer(keyBuffer);
}

void Input::OnChar(char character) noexcept
{
   charBuffer.push(character);
   TrimBuffer(charBuffer);
}

std::optional<Input::KeyEvent> Input::ReadKey() noexcept
{
   return std::optional<KeyEvent>();
}

std::optional<char> Input::ReadChar() noexcept
{
   if (charBuffer.size() > 0u)
   {
      unsigned char charcode = charBuffer.front();
      charBuffer.pop();
      return charcode;
   }
   return {};
}

void Input::OnMouseMove(int newx, int newy) noexcept
{
   x = newx;
   y = newy;

   mouseBuffer.push(Input::MouseEvent(Input::MouseEvent::MouseType::Move, *this));
   TrimBuffer();
}

void Input::OnMouseLeave() noexcept
{
   isInWindow = false;
   mouseBuffer.push(Input::MouseEvent(Input::MouseEvent::MouseType::Leave, *this));
   TrimBuffer();
}

void Input::OnMouseEnter() noexcept
{
   isInWindow = true;
   mouseBuffer.push(Input::MouseEvent(Input::MouseEvent::MouseType::Enter, *this));
   TrimBuffer();
}

void Input::OnLeftPressed(int x, int y) noexcept
{
   leftIsPressed = true;

   mouseBuffer.push(Input::MouseEvent(Input::MouseEvent::MouseType::LPress, *this));
   TrimBuffer();
}

void Input::OnLeftReleased(int x, int y) noexcept
{
   leftIsPressed = false;

   mouseBuffer.push(Input::MouseEvent(Input::MouseEvent::MouseType::LRelease, *this));
   TrimBuffer();
}

void Input::OnRightPressed(int x, int y) noexcept
{
   rightIsPressed = true;

   mouseBuffer.push(Input::MouseEvent(Input::MouseEvent::MouseType::RPress, *this));
   TrimBuffer();
}

void Input::OnRightReleased(int x, int y) noexcept
{
   rightIsPressed = false;

   mouseBuffer.push(Input::MouseEvent(Input::MouseEvent::MouseType::RRelease, *this));
   TrimBuffer();
}

void Input::OnWheelUp(int x, int y) noexcept
{
   mouseBuffer.push(Input::MouseEvent(Input::MouseEvent::MouseType::WheelUp, *this));
   TrimBuffer();
}

void Input::OnWheelDown(int x, int y) noexcept
{
   mouseBuffer.push(Input::MouseEvent(Input::MouseEvent::MouseType::WheelDown, *this));
   TrimBuffer();
}

void Input::TrimBuffer() noexcept
{
   while (mouseBuffer.size() > bufferSize)
   {
      mouseBuffer.pop();
   }
}

void Input::OnWheelDelta(int x, int y, int delta) noexcept
{
   wheelDeltaCarry += delta;
   // generate events for every 120 
   while (wheelDeltaCarry >= WHEEL_DELTA)
   {
      wheelDeltaCarry -= WHEEL_DELTA;
      OnWheelUp(x, y);
   }
   while (wheelDeltaCarry <= -WHEEL_DELTA)
   {
      wheelDeltaCarry += WHEEL_DELTA;
      OnWheelDown(x, y);
   }
}


template<typename T>
inline void Input::TrimBuffer(std::queue<T> &buffer) noexcept
{
   while (buffer.size() > bufferSize)
   {
      buffer.pop();
   }
}
