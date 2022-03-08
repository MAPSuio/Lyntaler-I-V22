void function(int x) {
  if (x > 1) { // find this
  }

  if (x > 1) { // not this
  } else {
  }

  if (x > 1) { // find this
  } else if (x < 2) { // and not this, etc.
  }

  if (x > 1) { // find this
  } else if (x < 2) { // and not this, etc.
  } else if (x > 2) { // and not this, etc.
  }

  if (x) { // find this
    if (x == 1) { // find this

    }
  }

  if (x) // find this
    if (x == 1) // find this
      (void) x;

  if (x) // not this
    (void) x;
  else
    (void) x;

  return;
}
