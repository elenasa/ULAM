//{

  typedef Unsigned(3) Dir;

  Coord x = 0;
  Coord y = 0;

  Self(Coord ax, Coord ay) {
    x = ax;
    y = ay;
  }

  Self(Self s) {
    x = s.x;
    y = s.y;
  }

  Void set(Int ax, Int ay) {
    x = (Coord) ax;
    y = (Coord) ay;
  }

  Void set(Self c) {
    set(c.x, c.y);
  }

  Self & operator=(Self other) {
    set(other);
    return self;
  }

  Self & operator+=(Self other) {
    // This single statement should work..
    // return self = self + other;  
    self = self + other;  
    return self;
  }

  Self & operator-=(Self other) {
    self = self - other;
    return self;
  }

  Self clamp(Self below, Self above) {
    return (self | below) & above;
  }

  Int getX() { return (Int) x; }
  Int getY() { return (Int) y; }

  Void setX(Int ax) { x = (Coord) ax; }
  Void setY(Int ay) { y = (Coord) ay; }

  Self make(Int x, Int y)
  {
    Self ret;
    ret.set(x,y);
    return ret;
  }

  Bool equals(Self other) {
    return x == other.x && y == other.y;
  }

  Bool operator==(Self other) {
    return equals(other);
  }

  Bool operator!=(Self other) {
    return !equals(other);
  }

  Self max(Self other) {
    if (other.x < self.x) other.x = self.x;
    if (other.y < self.y) other.y = self.y;
    return other;
  } 

  Self operator|(Self other) {
    return max(other);
  }

  Self& operator|=(Self other) {
    self = self | other;
    return self;
  }

  Self min(Self other) {
    if (other.x > self.x) other.x = self.x;
    if (other.y > self.y) other.y = self.y;
    return other;
  } 

  Self operator&(Self other) {
    return min(other);
  }

  Self& operator&=(Self other) {
    self = self & other;
    return self;
  }

  /** True if each dimension of \c other is between 0 and our
      dimension's value - 1. Always false if self.x or self.y
      is <= 0.  

      \since v2.1
   */
  Bool contains(Self other) {
    Int sx = self.getX();
    Int sy = self.getY();
    Int ox = other.getX();
    Int oy = other.getY();
    if (ox < 0 || ox >= sx) return false;
    if (oy < 0 || oy >= sy) return false;
    return true;
  }

  Int dot(Self other) {
    return x * other.x + y * other.y;
  }

  Int operator*(Self other) {
    return dot(other);
  }

  Self operator*(Coord scalar) {
    return make((Coord) (x * scalar), (Coord) (y * scalar));
  }

  Self operator/(Coord scalar) {
    return make((Coord) (x / scalar), (Coord) (y / scalar));
  }

  Self add(Self other) {
    return make((Coord) (x + other.x), (Coord) (y + other.y));
  }

  Self operator+(Self other) {
    return add(other);
  }

  Self subtract(Self other) {
    return make((Coord) (x - other.x), (Coord) (y - other.y));
  }

  Self operator-(Self other) {
    return subtract(other);
  }

  Self negate() {
    return make((Coord) -(Int) x, (Coord) -(Int) y);
  }

  Self operator-() {
    return negate();
  }

  Self operator+() {
    return self;
  }

  /**
     Return the von Neumann neighbor in the direction d
   */
  Self neighbor(Dir d) {
    if (d==0) return make( 0, 1);
    if (d==1) return make( 1, 1);
    if (d==2) return make( 1, 0);
    if (d==3) return make( 1,-1);
    if (d==4) return make( 0,-1);
    if (d==5) return make(-1,-1);
    if (d==6) return make(-1, 0);
    /* d==7*/ return make(-1, 1);
  }

  /**
     Return the event window point at distance 4 in the direction d.
   */
  Self corner(Dir d) {
    if (d==0) return make( 0, 4);
    if (d==1) return make( 2, 2);
    if (d==2) return make( 4, 0);
    if (d==3) return make( 2,-2);
    if (d==4) return make( 0,-4);
    if (d==5) return make(-2,-2);
    if (d==6) return make(-4, 0);
    /* d==7*/ return make(-2, 2);
  }

  Unsigned length() {
    Self origin;
    return origin.manhattanDistance(self);
  }

  Bool isOrigin() {
    return x == 0 && y == 0;
  }

  Self abs() {
    return abs(self);
  }

  Self abs(Self of) {
    if (of.x < 0) of.x = -of.x;
    if (of.y < 0) of.y = -of.y;
    return of;
  }

  Unsigned manhattanDistance(Self other) {
    Self a = abs(self-other);
    return (Unsigned) (a.x + a.y);
  }

  /** 
    \since v2.1
  */
  Unsigned euclideanSquaredDistance(Self other) {
    Self diff = self-other;
    return diff.euclideanSquaredLength();
  }

  /** 
    \since v2.1
  */
  Unsigned euclideanSquaredLength() {
    return (Unsigned) (self * self);
  }

//}
