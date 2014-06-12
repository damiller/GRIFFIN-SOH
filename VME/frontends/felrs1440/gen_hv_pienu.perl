#!/usr/bin/perl -w

my $confOdb = "odbedit";
my $confEq = "/Equipment/LRS1440";

my %names = readOdbArray("$confEq/Settings/Names");

my %xgroups;

#undef %names;
#$names{1} = "A%B%C1%D1";
#$names{2} = "A%B%C1%D2";
#$names{3} = "A%B%C2%D1";
#$names{4} = "A%B%C2%D2";

my $maxDepth = 0;

foreach my $k (keys %names)
  {
    my $name = $names{$k};

    my @n = split(/\%/, $name);

    my $depth = scalar @n;
    $maxDepth = $depth     if ($depth > $maxDepth);

    my $xg = "";
    while (1)
      {
	my $n = shift @n;
	last if ! defined $n;

	my $xxg = $xg;

	$xg .= "%" if length $xg>0;

	my $xr = "$xg$n";

	if (defined $n[0])
	  {
	    #print "-[$n] [$xxg] -> [$xr]\n";
	    $xgroups{$xxg}{$xr} = 1;
	  }
	else
	  {
	    #print "-[$n] [$xxg] -> [$name]\n";
	    $xgroups{$xxg}{$k} = 1;
	  }

	$xg .= $n;
      }
  }

if (0)
  {
    foreach my $g (sort keys %xgroups)
      {
	print "$g: ";
	my @gg = sort keys %{$xgroups{$g}};
	print scalar @gg;
	foreach my $gg (@gg)
	  {
	    print " $gg ";
	  }
	
	my $cnt = countGroup($g);
	print ", Count $cnt";
	print "\n";
      }
  }

#die "Here!";

my $table = "\n";
my $style = "\n";
my $script = "\n";
my %script;
my $ig = 0;

if (1)
  {
    $table .= "<table border=1 cellpadding=2>\n";

    $table .= "<tr>\n";
    $table .= "<th colspan=$maxDepth>Name<th>HWCH<th>switch<th>status<th>maxv<th>demand<th>adjust<th>measured\n";
    $table .= "</tr>\n";

    my $g = "";

    my @gg = sort keys %{$xgroups{$g}};

    $scriptIndex{$g} = $ig;
    $ig++;

    foreach my $gg (@gg)
      {
	outputGroup("", $gg, $maxDepth);

	my $ii = $scriptIndex{$gg};
	$script{$g} .= "  setSwitch_$ii(value); // group $gg\n" if defined $ii;
      }

    $table .= "</table>\n";
  }


if (1)
  {
    foreach my $k (sort keys %script)
      {
	#print "[$k]\n";
	my $ik = $scriptIndex{$k};
	$script .= "function setSwitch_$ik(value) { // group $k\n";
	$script .= "  clearTimeout(reloadTimerId);\n";

	$script .= $script{$k};
	$script .= "}\n\n";
      }

    $script .= "function setSwitch(value) { // top level\n";
    $script .= "  setSwitch_0(value);\n";
    $script .= "}\n\n";
  }

my $webpage = <<EOF
<html>
  <head>
    <title>PIENU LRS1440 High Voltage control</title>
    <!-- <meta http-equiv="Refresh" content="5">   -->

  </head>

  <body>
    <form name="form1" method="Get">
      <table border=1 cellpadding=2 width="100%">
	<tr><th colspan=2>PIENU LRS1440 High Voltage Control</tr>
	<tr><td width="50%">
            <input value="Status"    name="cmd" type="submit">
            <input value="ODB"       name="cmd" type="submit">
            <input value="History"   name="cmd" type="submit">
          </td><td>
            <center> <a href="http://midas.triumf.ca"> >>>> Midas Help <<<< </a></center>
          </td></tr>
      </tr>
      </table>
    </form>

    <script src='mhttpd.js'></script>

    <script>
      var reloadTimerId = 0;
    </script>

    <script>
      $script
    </script>

    <style type="text/css">
      td.mf_color { background-color: <odb src=\"$confEq/Status/statusColor\"> }
      $style
    </style>

    <form name="form2" method="Get">
      <hr>
      <table border=1 cellpadding=2">
        <tr>
          <td>LRS1440 Main switch: <odb src="$confEq/Settings/mainSwitch" edit=1> / <odb src="$confEq/Status/hvon"></td>
          <td class=mf_color>Status: <odb src="$confEq/Status/Status"> - <odb src="$confEq/Status/StatusMessage"></td>
          <td>Control enabled: <odb src="$confEq/Settings/EnableControl"></td>
          <td><input type=button value="turn on and ramp up" onClick="setSwitch(1); window.location.reload();"></input></td>
          <td><input type=button value="ramp down and turn off" onClick="setSwitch(0); window.location.reload();"></input></td>
          <!--- <td>Set voltages: <input type=input size=7 value="0" onKeyPress="if (event.keyCode==13) { setVoltage(this.value); window.location.reload() }"></input></td> --->
        </tr>
      </table>
      <hr>
      $table
    </form>

    <script>
      //alert("Hello!");
      reloadTimerId = setTimeout('window.location.reload()', 10000);
    </script>

  </body>
</html>
EOF
;

open(OUT, ">hv_pienu.html");
print OUT $webpage;
close OUT;

exit 0;

sub countGroup
  {
    my $g = shift @_;

    my @gg = sort keys %{$xgroups{$g}};
    my $nn = scalar @gg;

    my $count = 1;

    if ($nn > 0)
      {
	foreach my $gg (@gg)
	  {
	    $count += countGroup($gg);
	  }

	return $count;
      }
    else
      {
	return 1;
      }
  }

sub outputGroup
  {
    my $top = shift @_;
    my $g   = shift @_;
    my $depth = shift @_;

    my @gg = sort keys %{$xgroups{$g}};
    #my $nn = $xgroups{$g};  #scalar @gg;

    my $nn = countGroup($g);

    if (! defined $names{$g})
      {
	#print "group $g: $nn\n";

	my $n1 = $nn;


	my $tr = "";
	$tr .= "<tr>\n";
	$tr .= "<td rowspan=$n1 align=center>$g";
	$tr .= "<br>";
	$tr .= "<input type=button value=\"on\" onClick=\"setSwitch_$ig(1); window.location.reload();\"></input>";
	$tr .= "<input type=button value=\"off\" onClick=\"setSwitch_$ig(0); window.location.reload();\"></input>";
        $tr .= "\n";
        $tr .= "</tr>\n";
        $tr .= "\n";

	$table .= $tr;

	$scriptIndex{$g} = $ig;
	$ig++;

	foreach my $gg (@gg)
	  {
	    outputGroup($g, $gg, $depth-1);

	    my $ii = $scriptIndex{$gg};
	    $script{$g} .= "  setSwitch_$ii(value); // group $gg\n" if (defined $ii);
	  }
      }
    else
      {
	my $k = $g;
	my $e = $names{$k};

	my $tr = "";
	$tr .= "<tr class=style$k>\n";
	$tr .= "<td colspan=$depth align=center>$e</td>\n";
	my $slot = int($k/16);
	my $chan = $k%16;
	$tr .= "<td align=center>[$k] $slot-$chan</td>\n";
	$tr .= "<td align=center><odb src=\"$confEq/Settings/outputSwitch[$k]\" edit=1></td>\n";
	$tr .= "<td align=center><odb src=\"$confEq/Status/outputStatus[$k]\"></td>\n";
	$tr .= "<td align=center><odb src=\"$confEq/Status/MaxVoltage[$k]\"></td>\n";
	$tr .= "<td align=center><odb src=\"$confEq/Settings/outputVoltage[$k]\" edit=1> / <odb src=\"$confEq/Variables/Demand[$k]\"></td>\n";
        $tr .= "<td align=center><odb src=\"$confEq/Settings/MeasuredAdjust[$k]\" edit=1> V</td>\n";
        $tr .= "<td align=center><odb src=\"$confEq/Variables/Measured[$k]\"> V</td>\n";
	$tr .= "</tr>\n";
	$tr .= "\n";

	$table .= $tr;

	$style .= "tr.style$k { background-color: <odb src=\"$confEq/Status/outputColor[$k]\"> }\n";

	$script{$top} .= "  ODBSet(\'$confEq/Settings/outputSwitch[$k]\', value);\n";
      }
  }

sub xcmp
  {
    my $a = shift @_;
    my $b = shift @_;
    my $v = $names{$a} cmp $names{$b};
    #print "compare $names{$a} $names{$b} $v\n";
    return $v;
  }

sub readOdbArray
  {
    my $path = shift @_;
    my %a;

    open(IN, "$confOdb -c \"ls -l $path\" |");
    while (my $in = <IN>)
      {
	my ($i, $v) = $in =~ /\[(\d+)\]\s+(.*)\s+$/;
	#print "[$i] [$v] $in\n";
	next unless defined($i);
	next unless length($i)>0;
	$i = int($i);
	$a{$i} = $v;
      }
    close IN;

    return %a;
  }

# end
