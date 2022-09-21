package java.net;

import java.io.Serializable;
import java.util.Objects;

public class URI implements Comparable<URI>, Serializable {

  private static final long serialVersionUID = 1L;

  public static URI create(final String str) {
    try {
      return new URI(str);
    } catch (final URISyntaxException e) {
      throw new IllegalArgumentException(e);
    }
  }

  private static String buildURIString(final String scheme, final String ssp, final String fragment) {
    final StringBuilder sb = new StringBuilder();
    if (scheme != null) {
      sb.append(scheme);
      sb.append(':');
    }
    if (ssp != null)
      sb.append(quote(ssp));

    if (fragment != null)
      sb.append(quote(fragment));
    return sb.toString();
  }

  private static String buildURIString(
    final String scheme,
    final String userInfo,
    final String host,
    final int port,
    final String path,
    final String query,
    final String fragment
    ) {
    final StringBuilder sb = new StringBuilder();
    if (scheme != null) {
      sb.append(scheme);
      sb.append(':');
    }
    if (userInfo != null || host != null || port != -1) {
      sb.append("//");
    }
    if (userInfo != null) {
      sb.append(userInfo);
      sb.append('@');
    }
    if (host != null) {
      sb.append(host); /* check ipv6 literal */
    }
    if (port != -1) {
      sb.append(':');
      sb.append(port);
    }
    if (path != null) {
      sb.append(quote(path)); /* careful on / and @ */
    }
    if (query != null) {
      sb.append('?');
      sb.append(quote(query));
    }
    if (fragment != null) {
      sb.append('#');
      sb.append(fragment);
    }
    return sb.toString();
  }

  private static String buildURIString(
    final String scheme,
    final String authority,
    final String path,
    final String query,
    final String fragment
    ) {
    final StringBuilder sb = new StringBuilder();
    if (scheme != null) {
      sb.append(scheme);
      sb.append(':');
    }
    if (authority != null) {
      sb.append("//");
      /* TODO IPv6 literal chek then [] */
      sb.append(quote(authority)); /* careful on @ */
    }
    if (path != null) {
      sb.append(quote(path)); /* careful on / and @ */
    }
    if (query != null) {
      sb.append('?');
      sb.append(quote(query));
    }
    if (fragment != null) {
      sb.append('#');
      sb.append(fragment);
    }
    return sb.toString();
  }

  private static String quote(final String str) {
    // TODO Auto-generated method stub
    return str;
  }

  private String scheme, path, query, fragment, ssp;

  private String userInfo, authority;

  private String host;

  private int port;

  public URI(final String str) throws URISyntaxException {

  }

  public URI(final String nScheme, final String sSsp, final String nFragment) throws URISyntaxException {
    this(buildURIString(nScheme, sSsp, nFragment));
  }

  public URI(
    final String nScheme, final String nUserInfo, final String nHost, final int nPort, final String nPath,
    final String nQuery, final String nFragment
    ) throws URISyntaxException {
    this(buildURIString(nScheme, nUserInfo, nHost, nPort, nPath, nQuery, nFragment));
    /* parseServerAuthority? */
  }

  public URI(final String nScheme, final String nHost, final String nPath, final String nFragment)
    throws URISyntaxException {
    this(nScheme, null, nHost, -1, nPath, null, nFragment);
    /* parseServerAuthority? */
  }

  public URI(
    final String nScheme, final String nAuthority, final String nPath, final String nQuery, final String nFragment
    ) throws URISyntaxException {
    this(buildURIString(nScheme, nAuthority, nPath, nQuery, nFragment));
    /* parseServerAuthority() on the result? */
  }

  @Override
  public int compareTo(final URI o) {
    if (equals(o))
      return 0;
    return hashCode() - o.hashCode();
  }

  @Override
  public boolean equals(final Object obj) {
    if (this == obj)
      return true;
    if (obj == null)
      return false;
    if (getClass() != obj.getClass())
      return false;
    final URI other = (URI) obj;
    return Objects.equals(authority, other.authority) && Objects.equals(fragment, other.fragment)
      && Objects.equals(host, other.host) && Objects.equals(path, other.path) && port == other.port
      && Objects.equals(query, other.query) && Objects.equals(scheme, other.scheme) && Objects.equals(ssp, other.ssp)
      && Objects.equals(userInfo, other.userInfo);
  }

  public String getAuthority() {
    return authority;
  }

  public String getFragment() {
    return fragment;
  }

  public String getHost() {
    return host;
  }

  public String getPath() {
    return path;
  }

  public int getPort() {
    return port;
  }

  public String getQuery() {
    return query;
  }

  public String getRawAuthority() {
    return authority;
  }

  public String getRawFragment() {
    return fragment;
  }

  public String getRawPath() {
    return path;
  }

  public String getRawQuery() {
    return query;
  }

  public String getRawSchemeSpecificPart() {
    return ssp;
  }

  public String getRawUserInfo() {
    return userInfo;
  }

  public String getScheme() {
    return scheme;
  }

  public String getSchemeSpecificPart() {
    return ssp;
  }

  public String getUserInfo() {
    return userInfo;
  }

  @Override
  public int hashCode() {
    return Objects.hash(authority, fragment, host, path, port, query, scheme, ssp, userInfo);
  }

  public boolean isAbsolute() {
    return getScheme() != null;
  }

  public boolean isOpaque() {
    return isAbsolute() && getSchemeSpecificPart() != null && !getSchemeSpecificPart().startsWith("/");
  }

}
