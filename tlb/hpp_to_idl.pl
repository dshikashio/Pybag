#!/usr/bin/perl -w
use strict;
use warnings;

###########################################################
sub trace($)
{
	my ($trace_line) = @_;
	print "[INFO] $trace_line\n";
}

###########################################################
sub is_method_blacklisted($)
{
	my ($method) = @_;
	if ($method =~ /(QueryInterface|AddRef|Release)/)
	{
		return 1;
	}
	return 0;
}

###########################################################
sub replace_idl_types($$)
{
	my ($param_types, $iface_ptrs_ref) = @_;
	# remove attribute strings
	$param_types =~ s/(\sIN\s|__in\s|__out\s|\sOUT\s|__out_opt\s|OPTIONAL\s|__in_opt\s|__reserved\s|__inout\s)//g;
	
	# replace type declaration
	$param_types =~ s/PCSTR\s/const char* /g;
	$param_types =~ s/PSTR\s/char* /g;
	$param_types =~ s/PCWSTR\s/const wchar_t* /g;
	$param_types =~ s/PWSTR\s/wchar_t* /g;
	$param_types =~ s/PULONG\s/unsigned long* /g;
	$param_types =~ s/ULONG\s/unsigned long /g;
	$param_types =~ s/PULONG64\s/unsigned __int64* /g;
	$param_types =~ s/ULONG64\s/unsigned __int64 /g;
	$param_types =~ s/LONG64\s/__int64 /g;
	$param_types =~ s/PBOOL\s/bool* /g;
	$param_types =~ s/va_list\s/char* /g;
	$param_types =~ s/\.\.\.\s/SAFEARRAY(VARIANT) /g;
	$param_types =~ s/FARPROC/void*/g;

	# replace interface pointers
	foreach (keys %$iface_ptrs_ref)
	{
		$param_types =~ s/$_([\s\*])/$$iface_ptrs_ref{$_}\*$1/g;
	}
		
	return $param_types;
}

###########################################################
sub clean_params($$)
{
	my ($params, $iface_ptrs_ref) = @_;

	# remove whitespace
	$params =~ s/(\n|\r)//ig;
	$params =~ s/\s\s+/ /ig;

	# remove THIS_/THIS
	$params =~ s/THIS(_|)//;

	my @clean_params;
	my $optional_found_once = 0;
	foreach my $current_param (split(/,/, $params))
	{
		my @attr_strings;
		push @attr_strings, "in" if ($current_param =~ /(IN\s|__in\s|__in_opt\s|__inout\s|PVOID\s+Buffer|PULONG\s+BytesRead|PDEBUG_STACK_FRAME\s+Frames|PSTR\s+NameBuffer|PULONG64\s+Displacement)/);
		push @attr_strings, "out" if ($current_param =~ /(OUT\s|__out\s|__out_opt\s|__inout\s)/);

		$optional_found_once = 1 if ($current_param =~ /(OPTIONAL\s|__out_opt\s|__in_opt\s)/);
		push @attr_strings, "optional" if $optional_found_once;

		# replace type declaration
		$current_param = replace_idl_types($current_param, $iface_ptrs_ref);
		
		if (scalar @attr_strings)
		{
			$current_param = "[" . join(',', @attr_strings) . "]" . $current_param;
		}

		push @clean_params, $current_param;
	}

	return join(',', @clean_params);
}

###########################################################
sub print_idl_interface($$$$$$)
{
	my ($dst, $iface_name, $methods_array_ref, $method_params_ref, $iface_iids_ref, $iface_ptrs_ref) = @_;

	die "interface iid not found" unless (exists($$iface_iids_ref{$iface_name}));

	# print header
	print	$dst	"///////////////////////////////////////////////////////////\n".
					"[\n".
					"\tobject,\n".
					"\tuuid($$iface_iids_ref{$iface_name}),\n".
					"\thelpstring(\"$iface_name\")\n".
					"]\n".
					"interface $iface_name : IUnknown {\n";

	# print methods
	foreach my $method (@$methods_array_ref)
	{
		next if (is_method_blacklisted($method));
		my $params = clean_params($$method_params_ref{$method}, $iface_ptrs_ref);

		print $dst "\tHRESULT $method($params);\n";
	}

	# print footer
	print $dst "};\n\n";
}

###########################################################
sub strip_methods($$$)
{
	my ($iface_string, $methods_array_ref, $method_params_ref) = @_;

	# remove comments
	$iface_string =~ s/\/\*.*\*\// /ig;
	$iface_string =~ s/\/\/.*\n//ig;
	$iface_string =~ s/__(in|out)_(b|e)count(_opt|)\([^\)]+\)/__$1$3/ig;

	# split methods
	foreach (split(/;/, $iface_string))
	{
		# get method
		if (/\(([^\)]+)\).*\(([^\)]*)\)/)
		{
			push @$methods_array_ref, $1;
			$$method_params_ref{$1} = $2;
		}
	}
}

###########################################################
sub read_block($$)
{
	my ($src, $break_rx) = @_;
	my $block = "";
	while(<$src>)
	{
		$block .= $_;
		last if (/$break_rx/);
	}
	return $block;
}

###########################################################
my $src_file  = shift || die "missing src file";
my $dst_file  = shift || die "missing dst file";
my $interface = shift || undef;

trace("src file:  $src_file");
trace("dst file:  $dst_file");
trace("interface: $interface");

my $src;
open($src, "$src_file") || die "failed to open src file ($src_file)";

my $dst;
open($dst, ">$dst_file") || die "failed to create dst file ($dst_file)";

my %iface_iids;
my %iface_ptrs;
while(<$src>)
{
    if (/#define\s+([^\(]+)\s+((0x|)[0-9a-fA-F]+)\s+(.*)/)
    {
		if ($1 ne "INTERFACE")
		{
			trace("found define declaration. $1 = $2");
            # Constants bigger than 10 are rejected by midl
            if (length($2) <= 10)
            {
                print $dst "enum {$1 = $2};\n";
            }
			next;
		}
    }

	if (/typedef struct.*;/)
	{
		trace("found struct declaration (one liner)");
		print $dst $_;
		next;
	}
	
	if (/typedef struct[^;]+/)
	{
		trace("found struct declaration");

		my $brackets_count = 0;
		while(defined($_))
		{
			my $clean_field = replace_idl_types($_, \%iface_ptrs);
			print $dst $clean_field;
			
			++$brackets_count if (/{/);
			if (/}.*;/)
			{
				--$brackets_count;
				last if ($brackets_count == 0);
			}
			
			$_ = <$src>
		}
	}

	if (/typedef interface DECLSPEC_UUID\(\"([^\"]+)\"/)
	{
		my $iid = $1;
		
		# read whole string
		my $iid_string = read_block($src, ";");
		
		next unless ($iid_string =~ /(\w+)\s*\*\s+(\w+);/);
		my $iface_name = $1;
		my $iface_ptr = $2;
		trace("found interface id: $iface_ptr => $iface_name => $iid");
		
		$iface_ptrs{$iface_ptr} = $iface_name;
		$iface_iids{$iface_name} = $iid;
	}

	if (/DECLARE_INTERFACE_\((\w+)/)
	{
		my $iface_name = $1;
		trace("found interface: $iface_name");

		# check interface name
		next if (defined($interface) and ($iface_name ne $interface));
		
		# read whole interface declaration
		my $iface_string = read_block($src, "^};");

		# strip methods and their parameters
		my @iface_methods;
		my %method_params;
		
		trace("about to strip interface methods");
		strip_methods($iface_string, \@iface_methods, \%method_params);

		trace("got " . scalar @iface_methods . " methods");
		
		trace("about to print idl interface");
		print_idl_interface($dst, $iface_name, \@iface_methods, \%method_params, \%iface_iids, \%iface_ptrs);
	}
}

trace("done!");

close($dst);
close($src);
