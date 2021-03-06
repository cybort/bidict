.. _sorted-bidict-recipes:

Sorted Bidict Recipes
#####################

Suppose you need a bidict that maintains its items in sorted order.
The Python standard library does not include any sorted dict types,
but the excellent
`sortedcontainers <http://www.grantjenks.com/docs/sortedcontainers/>`_ and
`sortedcollections <http://www.grantjenks.com/docs/sortedcollections/>`_
libraries do.
Armed with these along with bidict's
:attr:`~bidict.BidictBase._fwdm_cls`
and
:attr:`~bidict.BidictBase._invm_cls`
attributes,
creating a sorted bidict type is dead simple::

    >>> import bidict, sortedcontainers

    >>> # a sorted bidict whose forward items stay sorted by their keys,
    >>> # and whose inverse items stay sorted by *their* keys (i.e. it and
    >>> # its inverse iterate over their items in different orders):

    >>> class SortedBidict(bidict.bidict):
    ...     _fwdm_cls = sortedcontainers.SortedDict
    ...     _invm_cls = sortedcontainers.SortedDict
    ...
    ...     # Include this for nicer repr's:
    ...     __repr_delegate__ = lambda x: list(x.items())

    >>> b = SortedBidict({'Tokyo': 'Japan', 'Cairo': 'Egypt'})
    >>> b
    SortedBidict([('Cairo', 'Egypt'), ('Tokyo', 'Japan')])

    >>> b['Lima'] = 'Peru'

    >>> # b stays sorted by its keys:
    >>> list(b.items())
    [('Cairo', 'Egypt'), ('Lima', 'Peru'), ('Tokyo', 'Japan')]

    >>> # b.inv stays sorted by *its* keys (b's values!)
    >>> list(b.inv.items())
    [('Egypt', 'Cairo'), ('Japan', 'Tokyo'), ('Peru', 'Lima')]


    >>> # a sorted bidict whose forward items stay sorted by their keys,
    >>> # and whose inverse items stay sorted by their values (i.e. it and
    >>> # its inverse iterate over their items in the same order):

    >>> import sortedcollections

    >>> class KeySortedBidict(bidict.bidict):
    ...     _fwdm_cls = sortedcontainers.SortedDict
    ...     _invm_cls = sortedcollections.ValueSortedDict
    ...
    ...     # Include this for nicer repr's:
    ...     __repr_delegate__ = lambda x: list(x.items())

    >>> element_by_atomic_number = KeySortedBidict({
    ...     3: 'lithium', 1: 'hydrogen', 2: 'helium'})

    >>> # stays sorted by key:
    >>> element_by_atomic_number
    KeySortedBidict([(1, 'hydrogen'), (2, 'helium'), (3, 'lithium')])

    >>> # .inv stays sorted by value:
    >>> list(element_by_atomic_number.inv.items())
    [('hydrogen', 1), ('helium', 2), ('lithium', 3)]

    >>> element_by_atomic_number[4] = 'beryllium'

    >>> list(element_by_atomic_number.inv.items())
    [('hydrogen', 1), ('helium', 2), ('lithium', 3), ('beryllium', 4)]

    >>> # This works because a bidict whose _fwdm_cls differs from its _invm_cls computes
    >>> # its inverse class -- which (note) is not actually the same class as the original,
    >>> # as it needs to have its _fwdm_cls and _invm_cls swapped -- automatically.
    >>> # You can see this if you inspect the inverse bidict:
    >>> element_by_atomic_number.inv  # Note the different class, which was auto-generated:
    KeySortedBidictInv([('hydrogen', 1), ('helium', 2), ('lithium', 3), ('beryllium', 4)])
    >>> ValueSortedBidict = element_by_atomic_number.inv.__class__
    >>> ValueSortedBidict._fwdm_cls
    <class 'sortedcollections.recipes.ValueSortedDict'>
    >>> ValueSortedBidict._invm_cls
    <class 'sortedcontainers.sorteddict.SortedDict'>

    >>> # Round trips work as expected:
    >>> atomic_number_by_element = ValueSortedBidict(element_by_atomic_number.inv)
    >>> atomic_number_by_element
    KeySortedBidictInv([('hydrogen', 1), ('helium', 2), ('lithium', 3), ('beryllium', 4)])
    >>> KeySortedBidict(atomic_number_by_element.inv) == element_by_atomic_number
    True

    >>> # One other useful trick:
    >>> # To pass method calls through to the _fwdm SortedDict when not present
    >>> # on the bidict instance, provide a custom __getattribute__ method:
    >>> def __getattribute__(self, name):
    ...     try:
    ...         return object.__getattribute__(self, name)
    ...     except AttributeError as e:
    ...         try:
    ...             return getattr(self._fwdm, name)
    ...         except AttributeError:
    ...             raise e

    >>> KeySortedBidict.__getattribute__ = __getattribute__

    >>> # bidict has no .peekitem attr, so the call is passed through to _fwdm:
    >>> element_by_atomic_number.peekitem()
    (4, 'beryllium')
    >>> element_by_atomic_number.inv.peekitem()
    ('beryllium', 4)
