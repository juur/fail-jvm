package java.util;

class ImplEntry<KK, VV> implements Map.Entry<KK, VV> {

  final KK          key;
  ImplEntry<KK, VV> next;

  VV value;

  public ImplEntry(final KK newKey, final VV newValue, final ImplEntry<KK, VV> newNext) {
    key = newKey;
    value = newValue;
    next = newNext;
  }

  @Override
  public KK getKey() {
    return key;
  }

  @Override
  public VV getValue() {
    return value;
  }

  @Override
  public VV setValue(final VV newValue) {
    value = newValue;
    return value;
  }
}