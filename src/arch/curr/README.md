# arch/curr directory

## Why does this directory exist

If i ever want to migrate to a different architecture, i need to separate architecture specific code.
And the other code needs to reference only current architecture (i planed to have symlink, but that only causes problems with build system).
And this approach alows us more control over what we can access form arch independent code.

## What are the contents

This directory should only include subset of headers that include header(with same name, so i dont go crazy) from currently selected architecture.

**Only headers that need to be exposed to arch independent code go here.**
