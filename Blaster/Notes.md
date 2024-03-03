# Notes

These notes basically exist for when I rewrite this using good practices, what things do I want to carry across.

For weapons:

* Some weapons have a Physics Asset which have parts that simulate physics (i.e. the SMG has a danging
  strap that dangles). So we want to be able to mark some weapons as simulating physics when we pick them
  up. (Although this may mean just setting collision enabled for physics)
