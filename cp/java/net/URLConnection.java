package java.net;

import java.io.IOException;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

public abstract class URLConnection {

  private static boolean defaultUseCaches = false;
  private static boolean defaultAllowUserInteraction = false;

  private boolean   allowUserInteraction;
  protected boolean connected;
  private boolean   doInput;
  private boolean   doOutput;
  private long      ifModifiedSince;
  protected URL     url;
  private boolean   useCaches;

  private final Map<String, List<String>> properties;

  protected URLConnection(final URL url) {
    this.url = url;
    properties = new HashMap<String, List<String>>();
    useCaches = defaultUseCaches;
    allowUserInteraction = defaultAllowUserInteraction;
  }

  public void addRequestProperty(final String key, final String value) {
    if (properties.containsKey(key))
      properties.get(key).add(value);
    else {
      final List<String> list = new ArrayList<String>();
      list.add(value);
      properties.put(key, list);
    }
  }

  public abstract void connect() throws IOException;

  public boolean getAllowUserInteraction() {
    return allowUserInteraction;
  }

  public boolean getDefaultAllowUserInteraction() {
    return defaultAllowUserInteraction;
  }

  public boolean getDoInput() {
    return doInput;
  }

  public boolean getDoOutput() {
    return doOutput;
  }

  public long getIfModifiedSince() {
    return ifModifiedSince;
  }

  public Map<String, List<String>> getProperties() {
    return properties;
  }

  public URL getUrl() {
    return url;
  }

  public boolean getUseCaches() {
    return useCaches;
  }

  public boolean isDefaultUseCaches() {
    return defaultUseCaches;
  }

  public void setAllowUserInteraction(final boolean allowuserinteraction) {
    allowUserInteraction = allowuserinteraction;
  }

  public void setDefaultAllowUserInteraction(final boolean defaultallowiserinteraction) {
    URLConnection.defaultAllowUserInteraction = defaultallowiserinteraction;
  }

  public void setDefaultUseCaches(final boolean defaultusecaches) {
    URLConnection.defaultUseCaches = defaultusecaches;
  }

  public void setDoInput(final boolean doInput) {
    this.doInput = doInput;
  }

  public void setDoOutput(final boolean doOutput) {
    this.doOutput = doOutput;
  }

  public void setIfModifiedSince(final long ifmodifiedsince) {
    ifModifiedSince = ifmodifiedsince;
  }

  public void setRequestProperty(final String key, final String value) {
    if (properties.containsKey(key))
      properties.remove(key);
    final String[] strs = value.split(",");
    for (final String str : strs)
      addRequestProperty(key, str);
  }

  public void setUseCaches(final boolean useCaches) {
    this.useCaches = useCaches;
  }
}
