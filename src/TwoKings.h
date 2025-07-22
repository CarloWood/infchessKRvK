#pragma once

class TwoKings
{
 private:
  Square bk_;
  Square wk_;

 public:
  TwoKings(Square const& bk, Square const& wk) : bk_(bk), wk_(wk) { }

  // Accessors.
  Square const& bk() const { return bk_; }
  Square const& wk() const { return wk_; }

  friend bool operator<(TwoKings const& lhs, TwoKings const& rhs)
  {
    return lhs.bk_.distance_less(rhs.bk_) || (!rhs.bk_.distance_less(lhs.bk_) && lhs.wk_.distance_less(rhs.wk_));
  }
};
